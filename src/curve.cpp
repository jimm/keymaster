#include "curve.h"
#include "generated_curves.h"

Curve curves[NUM_CURVE_SHAPES] = {
  {Linear, "Linear", linear_curve},
  {Exponential, "Exponential", exponential_curve},
  {InverseExponential, "Inverse Exponential", inverse_exponential_curve}
};

Curve * curve_with_shape(CurveShape shape) {
  return &curves[int(shape)];
}
