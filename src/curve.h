#ifndef CURVES_H
#define CURVES_H

#include <string>

#define NUM_CURVE_SHAPES 5

// The integer values of these enums are stored in the database, so don't
// reorder them without modifying how they're stored.
enum CurveShape {
  Linear,
  Exponential,
  InverseExponential,
  HalfExponential,
  HalfInverseExponential
};

struct Curve {
  CurveShape shape;
  std::string name;
  unsigned char *curve;         // 128 bytes
};

extern Curve curves[NUM_CURVE_SHAPES];

Curve * curve_with_shape(CurveShape shape);

#endif /* CURVES_H */
