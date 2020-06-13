#include "device.h"
#include "keymaster.h"

using namespace std;

// Key is PortMidi device id, value is PmDeviceInfo *.
map<int, const PmDeviceInfo *> pm_devices;

const map<int, const PmDeviceInfo *> &devices() {
  if (pm_devices.empty()) {
    int num_devices = Pm_CountDevices();
    for (int i = 0; i < num_devices; ++i)
      pm_devices[i] = Pm_GetDeviceInfo(i);
  }
  return pm_devices;
}

PmDeviceID find_device(const char *device_name, int device_type) {
  KeyMaster *km = KeyMaster_instance();

  if (km->testing)
    return pmNoDevice;

  for (auto &iter : pm_devices) {
    const PmDeviceInfo *info = iter.second;
    if (((device_type == 0 && info->input)
         || (device_type == 1 && info->output))
        && device_names_equal(device_name, (const char *)info->name))
      return iter.first;
  }
  return pmNoDevice;
}

/*
 * Case-insensitive string comparison that ignores leading and trailing
 * whitespace and returns 0 if they are equal. Assumes both strings are
 * non-NULL.
 */
bool device_names_equal(const char *name1, const char *name2) {
  while (isspace(*name1)) ++name1;
  while (isspace(*name2)) ++name2;

  const char *end1 = name1 + strlen(name1) - 1;
  while (end1 > name1 && isspace(*end1)) end1--;
  const char *end2 = name2 + strlen(name2) - 1;
  while (end2 > name2 && isspace(*end2)) end2--;

  int len1 = (int)(end1 - name1) + 1;
  int len2 = (int)(end2 - name2) + 1;

  if (len1 != len2)
    return false;
  return strncasecmp(name1, name2, len1) == 0;
}
