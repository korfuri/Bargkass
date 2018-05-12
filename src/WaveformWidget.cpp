#include "WaveformWidget.hpp"
#include "window.hpp"
#include <list>

WaveformWidget::WaveformWidget(Vec bounds, WFfn const& getWaveform) :
  bounds_(bounds), getWaveform_(getWaveform) {}

void WaveformWidget::setColor(NVGcolor const& color) {
  color_ = color;
}

void WaveformWidget::applyScissor(NVGcontext *vg) {
  nvgIntersectScissor(vg, 0.f, 0.0f, bounds_.x, bounds_.y);
}

void WaveformWidget::draw(NVGcontext *vg) {
  applyScissor(vg);
  WaveformDescriptor wf;
  getWaveform_(&wf);
  float x = bounds_.x;
  nvgBeginPath(vg);
  nvgMoveTo(vg, x, bounds_.y/2.f - *wf.rbegin);
  for (auto it = wf.rbegin; it != wf.rend; ++it) {
    x -= 1.0f;
    if (x < 0) {
      break;
    }
    nvgLineTo(vg, x, bounds_.y/2.0f - *it);
  }
  nvgStroke(vg);
  // Get the values from the module
  // Compute stats
  // Draw grid
  // Draw stats
  // Draw waveform
}
