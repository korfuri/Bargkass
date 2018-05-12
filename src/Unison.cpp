#include "Bargkass.hpp"
#include <random>

class Unison : public Module {
public:
  enum ParamIds {
    DETUNE_PARAM, // Range: +-1V (in V/oct)
    DETUNE_ATTV_PARAM, // Range: +-0.2. This attenuates DETUNE_INPUT
		       // which is clamped to +-5V, resulting in +-1V
		       // range like DETUNE_PARAM
    DELAY_PARAM, // Range: 0 to 1000, unit: ms.
    DELAY_ATTV_PARAM, // Range: 0 to +100, unit ms/V. Attenuates
		      // DELAY_INPUT which is clamped to 0 to +10V,
		      // resulting in +-1000ms, which is then clamped
		      // to positive only.
    NUM_PARAMS
  };
  enum InputIds {
    GATE_INPUT, // Gate signal. Schmitt thresholds: 0.1V, 1.7V.
    CV_INPUT, // CV, typically in V/oct but any range is accepted, and
	      // is unclamped throughout.
    DETUNE_INPUT, // +-5V, see DETUNE_ATTV_PARAM
    DELAY_INPUT, // 0V to +10V, see DELAY_ATTV_PARAM
    NUM_INPUTS
  };
  enum OutputIds {
    GATE1_OUTPUT, // All gates output 0V (off) or 5V (on)
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
    CV1_OUTPUT, // CV outs are not clamped, and preserve the original
    CV2_OUTPUT, // signal and any range of detune.
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
    GATE_IN_LIGHT, // Tracks the GATE_INPUT's schmitt trigger.
    GATE1_LIGHT, // These lights track GATE{n}_OUTPUT and have a
    GATE2_LIGHT, // boolean state.
    GATE3_LIGHT,
    GATE4_LIGHT,
    GATE5_LIGHT,
    GATE6_LIGHT,
    GATE7_LIGHT,
    GATE8_LIGHT,
    GATE9_LIGHT,
    GATE10_LIGHT,
    GATE11_LIGHT,
    GATE12_LIGHT,
    NUM_LIGHTS
  };

  Unison() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS), mt_(42) {}
  ~Unison() {}

  void step() override;
private:
  std::mt19937 mt_;
  SchmittTrigger gateIn_;
};

void Unison::step() {
  float cv = inputs[CV_INPUT].value;
  float detune = params[DETUNE_PARAM].value / 100.f;
  if (inputs[DETUNE_INPUT].active) {
    detune += clamp(inputs[DETUNE_INPUT].value, -5.f, 5.f) * params[DETUNE_ATTV_PARAM].value;
  }
  gateIn_.process(rescale(inputs[GATE_INPUT].value, 0.1f, 1.7f, 0.f, 1.f));
  float gateIn = gateIn_.isHigh() ? 5.0f : 0.0f;
  lights[GATE_IN_LIGHT].value = gateIn;

  float delay = params[DELAY_PARAM].value * (engineGetSampleRate() / 1000.f);
  if (inputs[DELAY_INPUT].active) {
    delay += clamp(inputs[DELAY_INPUT].value, 0.f, 10.f) * params[DELAY_ATTV_PARAM].value;
  }

  double p = 1./delay;
  std::bernoulli_distribution d(p);

  for (int n = 0; n < 12; ++n) {
    float expected_tune = cv + (6 - n) * detune;
    if (outputs[(int)GATE1_OUTPUT + n].value != gateIn || outputs[(int)CV1_OUTPUT + n].value != expected_tune) {
      if (d(mt_)) {
	outputs[(int)GATE1_OUTPUT + n].value = gateIn;
	outputs[(int)CV1_OUTPUT + n].value = expected_tune;
      }
    }
    lights[(int)GATE1_LIGHT + n].value = outputs[(int)GATE1_OUTPUT + n].value;
  }
}

static float line(int n) {
  return (float)(20 + 28*n);
}

#define COL_INPUTS 20
#define COL_CV 45
#define COL_GATE 75
#define COL_PARAMS COL_INPUTS
#define COL_LIGHT 100

#define HPORTSIZE 12
#define HHUGEKNOBSIZE 30
#define HKNOBSIZE 14
#define HATTVSIZE 11

struct UnisonWidget : ModuleWidget {
  UnisonWidget(Unison *module) : ModuleWidget(module) {
    setPanel(SVG::load(assetPlugin(plugin, "res/Unison.svg")));

    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    for (int l = 0; l < 12; ++l) {
      addOutput(Port::create<PJ301MPort>(Vec(COL_CV,   line(l)), Port::OUTPUT, module, (int)Unison::CV1_OUTPUT + l));
      addOutput(Port::create<PJ301MPort>(Vec(COL_GATE, line(l)), Port::OUTPUT, module, (int)Unison::GATE1_OUTPUT + l));
      addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(Vec(COL_LIGHT, line(l)), module, (int)Unison::GATE1_LIGHT + l));
    }

    addInput(Port::create<PJ301MPort>(Vec(COL_INPUTS - HPORTSIZE, 25), Port::INPUT, module, Unison::GATE_INPUT));
    addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(Vec(COL_INPUTS - HPORTSIZE + (COL_LIGHT - COL_GATE), 25), module, Unison::GATE_IN_LIGHT));
    addParam(ParamWidget::create<RoundBlackKnob>(Vec(COL_PARAMS - HKNOBSIZE, 60), module, Unison::DELAY_PARAM, 0.0f, 1000.0f, 0.0f));
    addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(COL_PARAMS - HATTVSIZE, 95), module, Unison::DELAY_ATTV_PARAM, 0.0f, 100.0f, 0.0f));
    addInput(Port::create<PJ301MPort>(Vec(COL_PARAMS - HPORTSIZE, 125), Port::INPUT, module, Unison::DELAY_INPUT));

    addInput(Port::create<PJ301MPort>(Vec(COL_INPUTS - HPORTSIZE, 225), Port::INPUT, module, Unison::CV_INPUT));
    addParam(ParamWidget::create<RoundBlackKnob>(Vec(COL_PARAMS - HKNOBSIZE, 260), module, Unison::DETUNE_PARAM, 0.0f, 1.0f, 0.0f));
    addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(COL_PARAMS - HATTVSIZE, 295), module, Unison::DETUNE_ATTV_PARAM, -0.2f, 0.2f, 0.0f));
    addInput(Port::create<PJ301MPort>(Vec(COL_INPUTS - HPORTSIZE, 325), Port::INPUT, module, Unison::DETUNE_INPUT));
  }
};

Model *modelUnison = Model::create<Unison, UnisonWidget>("Bargkass", "Unison", "Unison", MULTIPLE_TAG, EFFECT_TAG, DELAY_TAG);
