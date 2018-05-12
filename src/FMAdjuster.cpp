#include "Bargkass.hpp"
#include <algorithm>
#include <iostream>

class Adjuster : public Module {
public:
  enum ParamIds {
    ADJUST_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    VOCT_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    VOCT_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    // IN_LIGHT, // TODO
    // OUT_LIGHT, // TODO
    NUM_LIGHTS
  };

  Adjuster() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
  ~Adjuster() {}
  
  void step() override;
};

float adjustments[] = {
  -3.f, -2.f, -1.f, 0.f, +1.f, +2.f, +3.f
};
static const unsigned int nAdjustments = sizeof(adjustments)/sizeof(*adjustments);

void Adjuster::step() {
  outputs[VOCT_OUTPUT].value = inputs[VOCT_INPUT].value + adjustments[(int)params[ADJUST_PARAM].value];
}

struct AdjusterWidget : ModuleWidget {
  AdjusterWidget(Adjuster *module) : ModuleWidget(module) {
    setPanel(SVG::load(assetPlugin(plugin, "res/TestDisplay.svg"))); // TODO

    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(ParamWidget::create<Davies1900hBlackKnob>(Vec(28, 87), module, Adjuster::ADJUST_PARAM, 0.0f, (float)nAdjustments, (float)(nAdjustments/2)));

    addInput(Port::create<PJ301MPort>(Vec(11, 276), Port::INPUT, module, Adjuster::VOCT_INPUT));
    addOutput(Port::create<PJ301MPort>(Vec(11, 320), Port::OUTPUT, module, Adjuster::VOCT_OUTPUT));
  }
};

Model *modelAdjuster = Model::create<Adjuster, AdjusterWidget>("Bargkass", "Adjuster", "FM Adjuster", OSCILLATOR_TAG);
