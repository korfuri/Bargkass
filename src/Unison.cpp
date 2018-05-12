#include "Bargkass.hpp"
#include <random>

class Unison : public Module {
public:
  enum ParamIds {
    DETUNE_PARAM,
    DELAY_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    GATE_INPUT,
    CV_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    GATE1_OUTPUT,
    GATE2_OUTPUT,
    GATE3_OUTPUT,
    GATE4_OUTPUT,
    GATE5_OUTPUT,
    GATE6_OUTPUT,
    GATE7_OUTPUT,
    GATE8_OUTPUT,
    GATE9_OUTPUT,
    GATE10_OUTPUT,
    GATE11_OUTPUT,
    GATE12_OUTPUT,
    CV1_OUTPUT,
    CV2_OUTPUT,
    CV3_OUTPUT,
    CV4_OUTPUT,
    CV5_OUTPUT,
    CV6_OUTPUT,
    CV7_OUTPUT,
    CV8_OUTPUT,
    CV9_OUTPUT,
    CV10_OUTPUT,
    CV11_OUTPUT,
    CV12_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    NUM_LIGHTS
  };

  Unison() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS), mt_(42) {}
  ~Unison() {}
  
  void step() override;
private:
  std::mt19937 mt_;
};

void Unison::step() {
  float cv = inputs[CV_INPUT].value;
  for (int n = 0; n < 12; ++n) {
    outputs[(int)CV1_OUTPUT + n].value = cv + (6 - n) * params[DETUNE_PARAM].value / 100.f;
  }
  float gate = inputs[GATE_INPUT].value;
  double p = 1./(double)params[DELAY_PARAM].value;
  std::bernoulli_distribution d(p);
  for (int out = (int)GATE1_OUTPUT; out <= (int)GATE12_OUTPUT; ++out) {
    if (outputs[out].value != gate) {
      if (d(mt_)) {
	outputs[out].value = gate;
      }
    }
  }
}

static float line(int n) {
  return (float)(20 + 28*n);
}

#define COL_INPUTS 10
#define COL_CV 45
#define COL_GATE 75
#define COL_PARAMS 10

struct UnisonWidget : ModuleWidget {
  UnisonWidget(Unison *module) : ModuleWidget(module) {
    setPanel(SVG::load(assetPlugin(plugin, "res/Unison.svg")));

    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addInput(Port::create<PJ301MPort>(Vec(COL_INPUTS, line(0)), Port::INPUT, module, Unison::CV_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(COL_INPUTS, line(1)), Port::INPUT, module, Unison::GATE_INPUT));

    for (int l = 0; l < 12; ++l) {
      addOutput(Port::create<PJ301MPort>(Vec(COL_CV,   line(l)), Port::OUTPUT, module, (int)Unison::CV1_OUTPUT + l));
      addOutput(Port::create<PJ301MPort>(Vec(COL_GATE, line(l)), Port::OUTPUT, module, (int)Unison::GATE1_OUTPUT + l));
    }

    addParam(ParamWidget::create<RoundBlackKnob>(Vec(COL_PARAMS, line(4)), module, Unison::DELAY_PARAM, 1.0f, 44100.0f, 1.0f));
    addParam(ParamWidget::create<RoundBlackKnob>(Vec(COL_PARAMS, line(6)), module, Unison::DETUNE_PARAM, 0.0f, 3.0f, 0.0f));
  }
};

Model *modelUnison = Model::create<Unison, UnisonWidget>("Bargkass", "Unison", "Unison", MULTIPLE_TAG, EFFECT_TAG, DELAY_TAG);
