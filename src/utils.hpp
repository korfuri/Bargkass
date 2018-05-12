#ifndef BARGKASS_UTILS_HPP__
#define BARGKASS_UTILS_HPP__

#include "rack.hpp"
#include <cassert>

// TimeAccumulator keeps the time that elapsed since its creation. Its
// ::step() method should be called during each call to
// Module::step(). It remains accurate over time - no "512.0000f"
// upper limit (at 44100Hz) or "128.0000f" (at 192kHz) due to float
// resolution.
// 
class TimeAccumulator {
  double elapsed_;
  double maxLength_;
public:
  TimeAccumulator(double maxLength) : maxLength_(maxLength) {}

  double step(double duration = engineGetSampleTime()) {
    elapsed_ += duration;
    if (elapsed_ > maxLength_) {
      elapsed_ -= maxLength_;
    }
    return elapsed_;
  }
  
  double getElapsed() const {
    return elapsed_;
  }
};

struct ScrewFunk : public SVGScrew {
  ScrewFunk() {
    sw->setSVG(SVG::load(assetPlugin(plugin, "res/Screw.svg")));
    box.size = sw->box.size;
  }
};

#endif
