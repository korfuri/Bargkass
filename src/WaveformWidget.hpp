#include "rack.hpp"
#include <functional>
#include <vector>
#include <string>
#include <boost/circular_buffer.hpp>

struct WaveformDescriptor {
  boost::circular_buffer<float>::const_reverse_iterator rbegin, rend;
  float xmin, xmax;
  std::string unit;
};

using namespace rack;

class WaveformWidget : public TransparentWidget {
public:
  using WFfn = std::function<void(WaveformDescriptor*)>;
  WaveformWidget(Vec bounds, WFfn const&);

  template<typename  T>
  static T* create(const Rect& dim,
		   WFfn fn) {
		   //NVGcolor color = nvgRGBA(0, 0, 0, 255)
    auto t = new T(dim.size, fn);
    t->box.pos = dim.pos;
    return t;
  }

  void draw(NVGcontext *vg) override;
protected:
  void applyScissor(NVGcontext *vg);
  void setColor(NVGcolor const& color);
  
  Vec bounds_;
  WFfn const getWaveform_;
  NVGcolor color_;
};
