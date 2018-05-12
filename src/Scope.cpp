#include "Bargkass.hpp"
#include "WaveformWidget.hpp"
#include "utils.hpp"
#include <boost/circular_buffer.hpp>
#include <functional>
#include <vector>
#include <list>

struct Scope : Module {
  enum ParamIds {
    TIME_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    L_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    NUM_OUTPUTS
  };
  enum LightIds {
    NUM_LIGHTS
  };

  static const int NUM_SAMPLES = 5*192000; // 5s at 192kHz == 21s at 44.1kHz
  Scope() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS),
	    elapsed_(NUM_SAMPLES),
	    samples_(NUM_SAMPLES) {}

  void step() override;
  void getWaveform(WaveformDescriptor* d);

  //private:
  float timeResolution() const;
  
  TimeAccumulator elapsed_;
  boost::circular_buffer<float> samples_;
};

float Scope::timeResolution() const {
  return pow(2.0f, params[TIME_PARAM].value);
}

void Scope::step() {
  elapsed_.step();
  samples_.push_back(inputs[L_INPUT].value);
}

void Scope::getWaveform(WaveformDescriptor* d) {
  //return std::list<WaveformDescriptor>{WaveformDescriptor{samples_.end() - timeResolution(), samples_.end(), 0.0f, 1024.0f, std::string("V")}};
  d->rbegin = samples_.rbegin();
  d->rend = samples_.rend();
  d->xmin = 0.0f;
  d->xmax = 1024.0f;
  d->unit = "V";
}

struct ScopeWidget : ModuleWidget {
  ScopeWidget(Scope *module) : ModuleWidget(module) {
    using namespace std::placeholders;

    setPanel(SVG::load(assetPlugin(plugin, "res/Scope.svg")));

    addChild(Widget::create<ScrewFunk>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewFunk>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewFunk>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewFunk>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addChild(WaveformWidget::create<WaveformWidget>(Rect(Vec(30, 30), Vec(200, 200)), std::bind(&Scope::getWaveform, module, _1)));
    addInput(Port::create<PJ301MPort>(Vec(30, 300), Port::INPUT, module, Scope::L_INPUT));

    addParam(ParamWidget::create<Davies1900hBlackKnob>(Vec(100, 300), module, Scope::TIME_PARAM, 0.f, 10.f, 0.f));
  }
};

Model *modelScope = Model::create<Scope, ScopeWidget>("Bargkass", "Scope", "Scope module", BLANK_TAG);
