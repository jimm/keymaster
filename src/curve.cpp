#include "curve.h"
#include "formatter.h"
#include "generated_curves.h"
#include "keymaster.h"

void generate_default_curves(vector<Curve *> &vec) {
  Curve *curve;

  curve = new Curve(1, "Linear", "lin");
  memcpy(curve->curve, linear_curve, 128);
  vec.push_back(curve);

  curve = new Curve(1, "Exponential", "exp");
  memcpy(curve->curve, exponential_curve, 128);
  vec.push_back(curve);

  curve = new Curve(1, "Half Exponential", "exp/2");
  memcpy(curve->curve, half_exponential_curve, 128);
  vec.push_back(curve);

  curve = new Curve(1, "Inverse Exponential", "-exp");
  memcpy(curve->curve, inverse_exponential_curve, 128);
  vec.push_back(curve);

  curve = new Curve(1, "Half Inverse Exponential", "-exp/2");
  memcpy(curve->curve, half_inverse_exponential_curve, 128);
  vec.push_back(curve);
}

Curve::Curve(sqlite3_int64 id, const char *c_name, const char *c_short_name)
  : DBObj(id), Named(c_name), _short_name(c_short_name)
{
}

void Curve::from_chars(const char *str) {
  unsigned char *bytes = hex_to_bytes(str);
  memcpy(curve, bytes, 128);
  free(bytes);
}
