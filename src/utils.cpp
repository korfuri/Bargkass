#include "Bargkass.hpp"
#include <cmath>

float clamp(float f, float threshold) {
  if (f < -threshold) {
    return -threshold;
  }
  if (f > threshold) {
    return threshold;
  }
  if (std::isnan(f)) {
    return 0.0f;
  }
  return f;
}
