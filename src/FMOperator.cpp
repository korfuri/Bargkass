#include "Bargkass.hpp"
#include "AutoUI.hpp"
#include <algorithm>
#include <iostream>

// Damn compiler with no C++17 support
namespace std {
  inline float clamp(float v, float min, float max) {
    if (v < min) return min;
    if (v > max) return max;
    return v;
  }
}

class Operator : public Module {
public:
  // TODO: reorder ports and params here, when portlist is "final"
  enum ParamIds {
    unused_param0,
    MODE_PARAM, // Linear (0) or exponential (1) FM?
    FM_PARAM, // How much FM. Range: 0-100, scaled down to 0.0-1.0
    FM_CV_ATTV_PARAM, // Attenuverter for FM_CV_INPUT. Range +/-100, scaled down to +/-1
    DETUNE_PARAM, // Detune parameter in cents, -50 to +50
    DETUNE_CV_ATTV_PARAM, // Attenuverter for DETUNE_CV_INPUT. Range -100 to +100 cents
    OCTAVE_PARAM, // Octave adjuster, -5 to +5 octaves
    NUM_PARAMS
  };
  enum InputIds {
    GLOBAL_DETUNE_INPUT, // V/oct, base frequency offset from C4
    VOCT1_INPUT, // V/oct of the note 1 being played.
    FM1_INPUT, // Bimodal wave (-5/+5), carrier frequency.
    FM_CV_INPUT, // Bimodal CV (-5/+5), FM control.
    DETUNE_CV_INPUT, // Bimodal CV (-5/+5), detune control.
    ENVELOPE1_INPUT, // Unimodal (0/+10V) envelope control
    VOCT2_INPUT, // V/oct of the note 2 being played.
    VOCT3_INPUT, // V/oct of the note 3 being played.
    VOCT4_INPUT, // V/oct of the note 4 being played.
    ENVELOPE2_INPUT, // Unimodal (0/+10V) envelope control
    ENVELOPE3_INPUT, // Unimodal (0/+10V) envelope control
    ENVELOPE4_INPUT, // Unimodal (0/+10V) envelope control
    FM2_INPUT, // Bimodal wave (-5/+5), carrier frequency.
    FM3_INPUT, // Bimodal wave (-5/+5), carrier frequency.
    FM4_INPUT, // Bimodal wave (-5/+5), carrier frequency.
    NUM_INPUTS
  };
  enum OutputIds {
    GLOBAL_DETUNE_OUTPUT, // Passthrough of GLOBAL_DETUNE_INPUT
    VOCT1_OUTPUT, // Passthrough of VOCT1_INPUT,
    VOCT2_OUTPUT, // Passthrough of VOCT2_INPUT,
    VOCT3_OUTPUT, // Passthrough of VOCT3_INPUT,
    VOCT4_OUTPUT, // Passthrough of VOCT4_INPUT,
    WAVE1_OUTPUT, // Bimodal +/-5V wave 1 out
    WAVE2_OUTPUT, // Bimodal +/-5V wave 2 out
    WAVE3_OUTPUT, // Bimodal +/-5V wave 3 out
    WAVE4_OUTPUT, // Bimodal +/-5V wave 4 out
    NUM_OUTPUTS
  };
  enum LightIds {
    // ENVELOPE_ACTIVE_LIGHT, // TODO
    NUM_LIGHTS
  };

  //static constexpr const double A440 = 2.4494983453127;
  static constexpr const float C4 = 261.626f;
  
  Operator() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS), phase_{0., 0., 0., 0.} {}
  ~Operator() {}
  
  void step() override;
private:
  double phase_[4];
};

void Operator::step() {
  float detune = std::clamp(inputs[GLOBAL_DETUNE_INPUT].value, -5.f, 5.f)
    + params[OCTAVE_PARAM].value
    + params[DETUNE_PARAM].value / 100.f // The knob is +/-100, convert to +/-1V
    + params[DETUNE_CV_ATTV_PARAM].value * std::clamp(inputs[DETUNE_CV_INPUT].value, -5.f, 5.f) / 5.f / 12.f // convert +-5V to +-1semitone
    ;
  float fm_amount = (params[FM_PARAM].value / 100.f)
    + (params[FM_CV_ATTV_PARAM].value * std::clamp(inputs[FM_CV_INPUT].value, 0.f, 10.f) / 500.f);
  int mode = (int)params[MODE_PARAM].value;  // 1 for exponential, 0 for linear
  float pitch[4] = {
    // To use linear FM here we should log() the FM offset. Easier said than done since it can be negative. 
    detune + (mode ? std::clamp(inputs[FM1_INPUT].value, -5.f, 5.f) * fm_amount : 0.f) + std::clamp(inputs[VOCT1_INPUT].value, -5.f, 5.f),
    detune + (mode ? std::clamp(inputs[FM2_INPUT].value, -5.f, 5.f) * fm_amount : 0.f) + std::clamp(inputs[VOCT2_INPUT].value, -5.f, 5.f),
    detune + (mode ? std::clamp(inputs[FM3_INPUT].value, -5.f, 5.f) * fm_amount : 0.f) + std::clamp(inputs[VOCT3_INPUT].value, -5.f, 5.f),
    detune + (mode ? std::clamp(inputs[FM4_INPUT].value, -5.f, 5.f) * fm_amount : 0.f) + std::clamp(inputs[VOCT4_INPUT].value, -5.f, 5.f),
  };
  float envelope[4] = {
    inputs[ENVELOPE1_INPUT].active ? std::clamp(inputs[ENVELOPE1_INPUT].value, 0.f, 10.f) / 10.f : 1.f,
    inputs[ENVELOPE2_INPUT].active ? std::clamp(inputs[ENVELOPE2_INPUT].value, 0.f, 10.f) / 10.f : 1.f,
    inputs[ENVELOPE3_INPUT].active ? std::clamp(inputs[ENVELOPE3_INPUT].value, 0.f, 10.f) / 10.f : 1.f,
    inputs[ENVELOPE4_INPUT].active ? std::clamp(inputs[ENVELOPE4_INPUT].value, 0.f, 10.f) / 10.f : 1.f,
  };

  float freq[4] = {
    C4 * powf(2.0f, pitch[0]) + (mode ? 0.f : fm_amount * std::clamp(inputs[FM1_INPUT].value, -5.f, 5.f) * 400.f),
    C4 * powf(2.0f, pitch[1]) + (mode ? 0.f : fm_amount * std::clamp(inputs[FM2_INPUT].value, -5.f, 5.f) * 400.f),
    C4 * powf(2.0f, pitch[2]) + (mode ? 0.f : fm_amount * std::clamp(inputs[FM3_INPUT].value, -5.f, 5.f) * 400.f),
    C4 * powf(2.0f, pitch[3]) + (mode ? 0.f : fm_amount * std::clamp(inputs[FM4_INPUT].value, -5.f, 5.f) * 400.f),
  };
  
  // Passthroughs
  outputs[GLOBAL_DETUNE_OUTPUT].value = inputs[GLOBAL_DETUNE_INPUT].value;
  outputs[VOCT1_OUTPUT].value = inputs[VOCT1_INPUT].value;
  outputs[VOCT2_OUTPUT].value = inputs[VOCT2_INPUT].value;
  outputs[VOCT3_OUTPUT].value = inputs[VOCT3_INPUT].value;
  outputs[VOCT4_OUTPUT].value = inputs[VOCT4_INPUT].value;

  // Calculate the phase shift we're at. All functions are cyclical
  // with period 1 so we constrain to [0;1.0] here (and multiply by 2pi
  // later for sines).
  // If an envelope is provided and is 0, reset the phase.
  phase_[0] += freq[0] * engineGetSampleTime();
  if (phase_[0] > 1.0f) {
    phase_[0] -= 1.0f;
  }
  if (inputs[ENVELOPE1_INPUT].active && inputs[ENVELOPE1_INPUT].value <= 0.f) {
    phase_[0] = 0.f;
  }
  
  phase_[1] += freq[1] * engineGetSampleTime();
  if (phase_[1] > 1.0f) {
    phase_[1] -= 1.0f;
  }
  if (inputs[ENVELOPE2_INPUT].active && inputs[ENVELOPE2_INPUT].value <= 0.f) {
    phase_[1] = 0.f;
  }
  
  phase_[2] += freq[2] * engineGetSampleTime();
  if (phase_[2] > 1.0f) {
    phase_[2] -= 1.0f;
  }
  if (inputs[ENVELOPE3_INPUT].active && inputs[ENVELOPE3_INPUT].value <= 0.f) {
    phase_[2] = 0.f;
  }
  
  phase_[3] += freq[3] * engineGetSampleTime();
  if (phase_[3] > 1.0f) {
    phase_[3] -= 1.0f;
  }
  if (inputs[ENVELOPE4_INPUT].active && inputs[ENVELOPE4_INPUT].value <= 0.f) {
    phase_[3] = 0.f;
  }
  
  outputs[WAVE1_OUTPUT].value = sin(phase_[0] * 2 * M_PI) * 5.0f * envelope[0];
  outputs[WAVE2_OUTPUT].value = sin(phase_[1] * 2 * M_PI) * 5.0f * envelope[1];
  outputs[WAVE3_OUTPUT].value = sin(phase_[2] * 2 * M_PI) * 5.0f * envelope[2];
  outputs[WAVE4_OUTPUT].value = sin(phase_[3] * 2 * M_PI) * 5.0f * envelope[3];
}

// PJ301MPort: 32x32
// Davies1900 Large: 54x54
// Davies1900: 36x36
// RoundSmall: 30x
// Round: 38x
// RoundLarge: 48x
// RoundHuge: 72x

// H: 380
// W: 270
// HP: 15

#define LINE_DETUNE_GLOBAL 30
#define LINE_DETUNE_LOCAL 60
#define LINE_VOCT12 100
#define LINE_VOCT34 125
#define LINE_OCTAVE ((LINE_VOCT12 + LINE_VOCT34) / 2)
#define LINE_FM12 165
#define LINE_FM34 190
#define LINE_FM_CV 230
#define LINE_WAVE12 LINE_FM12
#define LINE_WAVE34 LINE_FM34
#define LINE_ENV12 310
#define LINE_ENV34 335
#define LINE_FM_KNOB 300
#define LINE_FM_MODE 340

#define COL_INPUTS_A 20
#define COL_INPUTS_B 45
#define COL_ATTV 50
#define COL_DETUNE_KNOB 130
#define COL_OCTAVE 85
#define COL_OUTPUTS_A 120
#define COL_OUTPUTS_B 145
#define COL_PARAMS_RIGHT 120

// Half diameter of round objects
#define HPORTSIZE 12
#define HHUGEKNOBSIZE 30
#define HKNOBSIZE 15
#define HATTVSIZE 11
#define HSWITCHSIZE 12

struct ExpLinSwitch : public SVGSwitch, ToggleSwitch {
  ExpLinSwitch() {
    addFrame(SVG::load(assetPlugin(plugin, "res/FMOperator_mode_lin.svg")));
    addFrame(SVG::load(assetPlugin(plugin, "res/FMOperator_mode_exp.svg")));
  }
};

struct OperatorWidget : ModuleWidget {
  OperatorWidget(Operator *module) : ModuleWidget(module) {
    setPanel(SVG::load(assetPlugin(plugin, "res/FMOperator.svg")));

    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    // Detune stuff
    addInput(Port::create<PJ301MPort>(Vec(COL_INPUTS_A - HPORTSIZE, LINE_DETUNE_GLOBAL - HPORTSIZE), Port::INPUT, module, Operator::GLOBAL_DETUNE_INPUT));
    addOutput(Port::create<PJ301MPort>(Vec(COL_OUTPUTS_B - HPORTSIZE, LINE_DETUNE_GLOBAL - HPORTSIZE), Port::OUTPUT, module, Operator::GLOBAL_DETUNE_OUTPUT));
    addInput(Port::create<PJ301MPort>(Vec(COL_INPUTS_A - HPORTSIZE, LINE_DETUNE_LOCAL - HPORTSIZE), Port::INPUT, module, Operator::DETUNE_CV_INPUT));
    addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(COL_ATTV - HATTVSIZE, LINE_DETUNE_LOCAL - HATTVSIZE), module, Operator::DETUNE_CV_ATTV_PARAM, -1.0f, 1.0f, 0.0f));
    addParam(ParamWidget::create<RoundBlackKnob>(Vec(COL_DETUNE_KNOB - HKNOBSIZE, LINE_DETUNE_LOCAL - HKNOBSIZE), module, Operator::DETUNE_PARAM, -100.0f, 100.0f, 0.0f));
    
    // CV inputs and outputs
    addInput(Port::create<PJ301MPort>(Vec(COL_INPUTS_A - HPORTSIZE, LINE_VOCT12 - HPORTSIZE), Port::INPUT, module, Operator::VOCT1_INPUT));    
    addOutput(Port::create<PJ301MPort>(Vec(COL_OUTPUTS_A - HPORTSIZE, LINE_VOCT12 - HPORTSIZE), Port::OUTPUT, module, Operator::VOCT1_OUTPUT));
    addInput(Port::create<PJ301MPort>(Vec(COL_INPUTS_B - HPORTSIZE, LINE_VOCT12 - HPORTSIZE), Port::INPUT, module, Operator::VOCT2_INPUT));
    addOutput(Port::create<PJ301MPort>(Vec(COL_OUTPUTS_B - HPORTSIZE, LINE_VOCT12 - HPORTSIZE), Port::OUTPUT, module, Operator::VOCT2_OUTPUT));
    addInput(Port::create<PJ301MPort>(Vec(COL_INPUTS_A - HPORTSIZE, LINE_VOCT34 - HPORTSIZE), Port::INPUT, module, Operator::VOCT3_INPUT));    
    addOutput(Port::create<PJ301MPort>(Vec(COL_OUTPUTS_A - HPORTSIZE, LINE_VOCT34 - HPORTSIZE), Port::OUTPUT, module, Operator::VOCT3_OUTPUT));
    addInput(Port::create<PJ301MPort>(Vec(COL_INPUTS_B - HPORTSIZE, LINE_VOCT34 - HPORTSIZE), Port::INPUT, module, Operator::VOCT4_INPUT));
    addOutput(Port::create<PJ301MPort>(Vec(COL_OUTPUTS_B - HPORTSIZE, LINE_VOCT34 - HPORTSIZE), Port::OUTPUT, module, Operator::VOCT4_OUTPUT));

    // Waves
    addOutput(Port::create<PJ301MPort>(Vec(COL_OUTPUTS_A - HPORTSIZE, LINE_WAVE12 - HPORTSIZE), Port::OUTPUT, module, Operator::WAVE1_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(COL_OUTPUTS_B - HPORTSIZE, LINE_WAVE12 - HPORTSIZE), Port::OUTPUT, module, Operator::WAVE2_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(COL_OUTPUTS_A - HPORTSIZE, LINE_WAVE34 - HPORTSIZE), Port::OUTPUT, module, Operator::WAVE3_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(COL_OUTPUTS_B - HPORTSIZE, LINE_WAVE34 - HPORTSIZE), Port::OUTPUT, module, Operator::WAVE4_OUTPUT));
    
    // FM
    addInput(Port::create<PJ301MPort>(Vec(COL_INPUTS_A - HPORTSIZE, LINE_FM12 - HPORTSIZE), Port::INPUT, module, Operator::FM1_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(COL_INPUTS_B - HPORTSIZE, LINE_FM12 - HPORTSIZE), Port::INPUT, module, Operator::FM2_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(COL_INPUTS_A - HPORTSIZE, LINE_FM34 - HPORTSIZE), Port::INPUT, module, Operator::FM3_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(COL_INPUTS_B - HPORTSIZE, LINE_FM34 - HPORTSIZE), Port::INPUT, module, Operator::FM4_INPUT));
    
    // Octaver
    addParam(ParamWidget::create<RoundBlackSnapKnob>(Vec(COL_OCTAVE - HKNOBSIZE, LINE_OCTAVE - HKNOBSIZE), module, Operator::OCTAVE_PARAM, -5.f, 5.f, 0.f));
 
    // FM CV
    addInput(Port::create<PJ301MPort>(Vec(COL_INPUTS_A - HPORTSIZE, LINE_FM_CV - HPORTSIZE), Port::INPUT, module, Operator::FM_CV_INPUT));
    addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(COL_ATTV - HPORTSIZE, LINE_FM_CV - HATTVSIZE), module, Operator::FM_CV_ATTV_PARAM, 0.0f, 100.0f, 0.0f));

    // FM Control
    addParam(ParamWidget::create<RoundHugeBlackKnob>(Vec(COL_PARAMS_RIGHT - HHUGEKNOBSIZE, LINE_FM_KNOB - HHUGEKNOBSIZE), module, Operator::FM_PARAM, 0.0f, 100.0f, 0.0f));
    addParam(ParamWidget::create<ExpLinSwitch>(Vec(COL_PARAMS_RIGHT - HSWITCHSIZE, LINE_FM_MODE - HSWITCHSIZE), module, Operator::MODE_PARAM, 0.0f, 1.0f, 0.0f));
    
    // Envelopes
    addInput(Port::create<PJ301MPort>(Vec(COL_INPUTS_A - HPORTSIZE, LINE_ENV12 - HPORTSIZE), Port::INPUT, module, Operator::ENVELOPE1_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(COL_INPUTS_B - HPORTSIZE, LINE_ENV12 - HPORTSIZE), Port::INPUT, module, Operator::ENVELOPE2_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(COL_INPUTS_A - HPORTSIZE, LINE_ENV34 - HPORTSIZE), Port::INPUT, module, Operator::ENVELOPE3_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(COL_INPUTS_B - HPORTSIZE, LINE_ENV34 - HPORTSIZE), Port::INPUT, module, Operator::ENVELOPE4_INPUT));
  }
};

Model *modelOperator = Model::create<Operator, OperatorWidget>("Bargkass", "Operator", "FM Operator", OSCILLATOR_TAG);
