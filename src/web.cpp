/*
 * Based on
 * http://www.cs.cmu.edu/afs/cs/academic/class/15213-s00/www/class28/tiny.c
 */

/*
 * tiny.c - a minimal HTTP server that serves static and
 *          dynamic content with the GET method. Neither
 *          robust, secure, nor modular. Use for instructional
 *          purposes only.
 *          Dave O'Hallaron, Carnegie Mellon
 *
 *          usage: tiny <port>
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "web.h"
#include "cursor.h"
#include "formatter.h"
#include "json.h"

#define BUFSIZE 1024
#define MAXERRS 16
#define EMPTY_STRING "\"\""

extern char **environ; /* the environment */

Web::Web(KeyMaster *keymaster, int port_number) : km(keymaster), port_num(port_number) {}

Web::~Web() {}

int Web::run() {
  /* variables for connection management */
  int parentfd;          /* parent socket */
  int childfd;           /* child socket */
  socklen_t clientlen;   /* byte size of client's address */
  struct hostent *hostp; /* client host info */
  char *hostaddrp;       /* dotted decimal host addr string */
  int optval;            /* flag value for setsockopt */
  struct sockaddr_in serveraddr; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */

  /* variables for connection I/O */
  char buf[BUFSIZE];     /* message buffer */
  char method[BUFSIZE];  /* request method */
  char uri[BUFSIZE];     /* request uri */
  char version[BUFSIZE]; /* request method */
  char filename[BUFSIZE];/* path derived from uri */
  char filetype[BUFSIZE];/* path derived from uri */
  char cgiargs[BUFSIZE]; /* cgi argument list */
  void *p;               /* temporary pointer */
  int is_static;         /* static request? */
  struct stat sbuf;      /* file status */
  int fd;                /* static content filedes */
  int pid;               /* process id from fork */
  int wait_status;       /* status from wait */

  if (errno != 0)
    error("ERROR oops before start!");

  /* open socket descriptor */
  parentfd = socket(AF_INET, SOCK_STREAM, 0);
  if (parentfd < 0)
    error("ERROR opening socket");

  /* allows us to restart server immediately */
  optval = 1;
  if (setsockopt(parentfd, SOL_SOCKET, SO_REUSEADDR,
                 (const void *)&optval , sizeof(int)) != 0)
    error("ERROR on setsockopt");

  /* bind port to socket */
  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons((uint16_t)port_num);
  bind(parentfd, (const struct sockaddr *)&serveraddr, (socklen_t)sizeof(serveraddr));
  if (errno != 0)
    error("ERROR on binding");

  /* get us ready to accept connection requests */
  if (listen(parentfd, 5) < 0) /* allow 5 requests to queue up */
    error("ERROR on listen");

  /*
   * main loop: wait for a connection request, parse HTTP,
   * serve requested content, close connection.
   */
  clientlen = sizeof(clientaddr);
  while (1) {

    /* wait for a connection request */
    childfd = accept(parentfd, (struct sockaddr *) &clientaddr, &clientlen);
    if (childfd < 0)
      error("ERROR on accept");

    /* determine who sent the message */
    hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,
			  sizeof(clientaddr.sin_addr.s_addr), AF_INET);
    if (hostp == NULL)
      error("ERROR on gethostbyaddr");
    hostaddrp = inet_ntoa(clientaddr.sin_addr);
    if (hostaddrp == NULL)
      error("ERROR on inet_ntoa\n");

    /* open the child socket descriptor as a stream */
    if ((stream = fdopen(childfd, "r+")) == NULL)
      error("ERROR on fdopen");

    /* get the HTTP request line */
    fgets(buf, BUFSIZE, stream);
    sscanf(buf, "%s %s %s\n", method, uri, version);

    /* tiny only supports the GET method */
    if (strcasecmp(method, "GET")) {
      cerror(method, 501, "Not Implemented", "Tiny does not implement this method");
      fclose(stream);
      close(childfd);
      continue;
    }

    /* read (and ignore) the HTTP headers */
    fgets(buf, BUFSIZE, stream);
    while(strcmp(buf, "\r\n"))
      fgets(buf, BUFSIZE, stream);

    /* parse the uri [crufty] */
    char *cgiargs_start = index(uri, '?');
    if (cgiargs_start != nullptr) {
      strncpy(cgiargs, cgiargs_start+1, BUFSIZE);
      cgiargs[BUFSIZE-1] = '\0';
      *cgiargs_start = '\0';
    }
    else
      cgiargs[0] = '\0';

    /* routing */
    if (strncmp(uri, "/status", 7) == 0) {
DONE_JSON:
      return_status();
CLOSE_STREAM_AND_RETURN:
      fclose(stream);
      close(childfd);
      continue;
    }
    if (strncmp(uri, "/next_patch", 11) == 0) {
      km->next_patch();
      goto DONE_JSON;
    }
    else if (strncmp(uri, "/prev_patch", 11) == 0) {
      km->prev_patch();
      goto DONE_JSON;
    }
    else if (strncmp(uri, "/next_song", 10) == 0) {
      km->next_song();
      goto DONE_JSON;
    }
    else if (strncmp(uri, "/prev_song", 10) == 0) {
      km->prev_song();
      goto DONE_JSON;
    }
    else if (strncmp(uri, "/song/edit", 10) == 0) {
      parse_params(cgiargs);
      km->cursor()->song()->set_name(params["songname"]);
REDIRECT_BACK_TO_INDEX:
      /* print response header */
      fprintf(stream, "HTTP/1.1 302 OK\n");
      fprintf(stream, "Server: Tiny Web Server\n");
      fprintf(stream, "Location: /\n");
      fprintf(stream, "Content-length: 0\n");
      fprintf(stream, "\r\n");
      fflush(stream);
      goto CLOSE_STREAM_AND_RETURN;
    }
    else if (strncmp(uri, "/patch/edit", 11) == 0) {
      parse_params(cgiargs);
      km->cursor()->song()->set_name(params["patchname"]);
      goto REDIRECT_BACK_TO_INDEX;
    }
    else {
      is_static = 1;
      strcpy(cgiargs, "");
      strcpy(filename, "./public");
      strcat(filename, uri);
      if (uri[strlen(uri)-1] == '/')
	strcat(filename, "index.html");
    }

    /* make sure the file exists */
    if (stat(filename, &sbuf) < 0) {
      cerror(filename, 404, "Not found", "Tiny couldn't find this file");
      fclose(stream);
      close(childfd);
      continue;
    }

    /* serve static content */
    if (is_static) {
      if (strstr(filename, ".html"))
	strcpy(filetype, "text/html");
      else if (strstr(filename, ".gif"))
	strcpy(filetype, "image/gif");
      else if (strstr(filename, ".jpg"))
	strcpy(filetype, "image/jpg");
      else if (strstr(filename, ".js"))
	strcpy(filetype, "application/javascript");
      else if (strstr(filename, ".css"))
	strcpy(filetype, "text/css");
      else
	strcpy(filetype, "text/plain");

      /* print response header */
      fprintf(stream, "HTTP/1.1 200 OK\n");
      fprintf(stream, "Server: Tiny Web Server\n");
      fprintf(stream, "Content-length: %d\n", (int)sbuf.st_size);
      fprintf(stream, "Content-type: %s\n", filetype);
      fprintf(stream, "\r\n");
      fflush(stream);

      /* Use mmap to return arbitrary-sized response body */
      fd = open(filename, O_RDONLY);
      p = mmap(0, sbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
      fwrite(p, 1, sbuf.st_size, stream);
      munmap(p, sbuf.st_size);
    }

    /* serve dynamic content */
    else {
      /* make sure file is a regular executable file */
      if (!(S_IFREG & sbuf.st_mode) || !(S_IXUSR & sbuf.st_mode)) {
	cerror(filename, 403, "Forbidden", "You are not allow to access this item");
	fclose(stream);
	close(childfd);
	continue;
      }

      /* a real server would set other CGI environ vars as well*/
      setenv("QUERY_STRING", cgiargs, 1);

      /* print first part of response header */
      sprintf(buf, "HTTP/1.1 200 OK\n");
      write(childfd, buf, strlen(buf));
      sprintf(buf, "Server: Tiny Web Server\n");
      write(childfd, buf, strlen(buf));

      /* create and run the child CGI process so that all child
         output to stdout and stderr goes back to the client via the
         childfd socket descriptor */
      pid = fork();
      if (pid < 0) {
	perror("ERROR in fork");
	exit(1);
      }
      else if (pid > 0) { /* parent process */
	wait(&wait_status);
      }
      else { /* child  process*/
	close(0); /* close stdin */
	dup2(childfd, 1); /* map socket to stdout */
	dup2(childfd, 2); /* map socket to stderr */
	if (execve(filename, NULL, environ) < 0) {
	  perror("ERROR in execve");
	}
      }
    }

    /* clean up */
    fclose(stream);
    close(childfd);

  }
}

/*
 * error - wrapper for perror used for bad syscalls
 */
void Web::error(const char *msg) {
  perror(msg);
  exit(1);
}

/*
 * cerror - returns an error message to the client
 */
void Web::cerror(const char *cause, int error_number,
	    const char *shortmsg, const char *longmsg)
{
  fprintf(stream, "HTTP/1.1 %d %s\n", error_number, shortmsg);
  fprintf(stream, "Content-type: text/html\n");
  fprintf(stream, "\n");
  fprintf(stream, "<html><title>Tiny Error</title>");
  fprintf(stream, "<body bgcolor=""ffffff"">\n");
  fprintf(stream, "%d: %s\n", error_number, shortmsg);
  fprintf(stream, "<p>%s: %s\n", longmsg, cause);
  fprintf(stream, "<hr><em>The Tiny Web server</em>\n");
}

void Web::return_status() {
  string str = status_json();

  fprintf(stream, "HTTP/1.1 200 OK\n");
  fprintf(stream, "Server: Tiny Web Server\n");
  fprintf(stream, "Content-length: %ld\n", str.size());
  fprintf(stream, "Content-type: application/json\n");
  fprintf(stream, "\r\n");
  fflush(stream);

  fwrite(str.c_str(), str.size(), 1, stream);
  fflush(stream);
}

string Web::status_json() {
  JSON json;
  json.encode(*km);
  return json.str();
}

string Web::unencode(const char *p) {
  string str = "";
  while (*p) {
    switch (*p) {
    case '%':
      str += (char)hex_to_byte(p+1);
      p += 2;
      break;
    case '+':
      str += ' ';
      break;
    default:
      str += *p;
      break;
    }
    ++p;
  }
  return str;
}

void Web::parse_params(const char *cgiargs) {
  params.clear();
  if (*cgiargs == '\0')
    return;

  char *token, *str, *to_free;

  to_free = str = strdup(cgiargs);
  while ((token = strsep(&str, "&")) != NULL) {
    char *equals = index(token, '=');
    if (equals == nullptr)
      params[unencode(token)] = "";
    else {
      *equals = '\0';
      params[unencode(token)] = unencode(equals + 1);
    }
  }
  free(to_free);
}
