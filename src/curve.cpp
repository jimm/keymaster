#include "curve.h"
#include "formatter.h"

class LinearCurve : public Curve {
public:
  LinearCurve() : Curve(1, "Linear", "lin") { generate(); }
protected:
  virtual void generate() {
    for (int i = 0; i < 128; ++i)
      curve[i] = i;
  }
};

class ExponentialCurve : public Curve {
public:
  ExponentialCurve() : Curve(1, "Exponential", "exp") { generate(); }
protected:
  virtual void generate() {
    for (int i = 0; i < 128; ++i) {
      int val = i * i / 127;
      if (i > 0 && val == 0)
        val = 1;
      curve[i] = val;
    }
  }
};

class HalfExponentialCurve : public Curve {
public:
  HalfExponentialCurve() : Curve(1, "Half Exponential", "exp/2") { generate(); }
protected:
  virtual void generate() {
    for (int i = 0; i < 128; ++i) {
      int linear = i;
      int exponential = i * i / 127;
      if (i > 0 && exponential == 0)
        exponential = 1;
      curve[i] = (linear + exponential) / 2;
    }
  }
};

class InverseExponentialCurve : public Curve {
public:
  InverseExponentialCurve() : Curve(1, "Inverse Exponential", "-exp") { generate(); }
protected:
  virtual void generate() {
    for (int i = 0; i < 128; ++i) {
      int val = 127 - i;
      val = 127 - ((val * val) / 127);
      curve[i] = val;
    }
  }
};

class HalfInverseExponentialCurve : public Curve {
public:
  HalfInverseExponentialCurve() : Curve(1, "Half Inverse Exponential", "-exp/2") { generate(); }
protected:
  virtual void generate() {
    for (int i = 0; i < 128; ++i) {
      int linear = i;
      int inv_exp = 127 - i;
      inv_exp = 127 - ((inv_exp * inv_exp) / 127);
      curve[i] = (linear + inv_exp) / 2;
    }
  }
};

void generate_default_curves(vector<Curve *> &vec) {
  vec.push_back(new LinearCurve());
  vec.push_back(new ExponentialCurve());
  vec.push_back(new HalfExponentialCurve());
  vec.push_back(new InverseExponentialCurve());
  vec.push_back(new HalfInverseExponentialCurve());
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
