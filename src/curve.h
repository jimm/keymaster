#ifndef CURVES_H
#define CURVES_H

#include <string>

#define NUM_CURVE_SHAPES 3

enum CurveShape {
  Linear,
  Exponential,
  InverseExponential
};

struct Curve {
  CurveShape shape;
  std::string name;
  unsigned char *curve;         // 128 bytes
};

extern Curve curves[NUM_CURVE_SHAPES];

Curve * curve_with_shape(CurveShape shape);

#endif /* CURVES_H */
