#include "Bargkass.hpp"
#include <cstdio>

struct Distortion : Module {
  enum ParamIds {
    FN_PARAM,
    CV_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    AUDIO_INPUT,
    CV_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    AUDIO_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    NUM_LIGHTS
  };
  
  Distortion() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
  void step() override;

  // For more advanced Module features, read Rack's engine.hpp header file
  // - toJson, fromJson: serialization of internal data
  // - onSampleRateChange: event triggered by a change of sample rate
  // - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};

static float offset(float f, float cv) {
  return f + cv;
}

static float fsin(float f, float cv) {
  return sin(f * cv * 50.0f);
}

static float ftanh(float f, float cv) {
  return tanh(f * cv);
}

static float powEven(float f, float cv) {
  return pow(f, pow(2, int(cv)));
}

static float powOdd(float f, float cv) {
  return pow(f, pow(2, int(cv)) + 1);
}

static float chebyshev(float x, float cv) {
  x = clamp(x, 1.0f);
  float last = x;
  float lastlast = 1.0f;
  float sum = x;
  unsigned int degree = (unsigned int)((cv + 5.0f) * 5.0f);
  for (unsigned int i = 0; i < degree; ++i) {
    float current = 2 * x * last - lastlast;
    sum += current;
    lastlast = last;
    last = current;
  }
  return sum;
}

typedef float (*fun_t)(float, float);

static fun_t tFns[] = {
  offset,
  fsin,
  ftanh,
  powOdd,
  powEven,
  chebyshev,
};
#define NUM_FNS (sizeof(tFns) / sizeof(tFns[0]))

void Distortion::step() {
  // If no input is fed, don't generate anything.
  if (!inputs[AUDIO_INPUT].active) {
    outputs[AUDIO_OUTPUT].value = 0.0f;
    return;
  }

  float cv = params[CV_PARAM].value;
  if (inputs[CV_INPUT].active) {
    cv = inputs[CV_INPUT].value;
  }

  float in = inputs[AUDIO_INPUT].value / 5.0f;
  unsigned int ifn = (unsigned int)(params[FN_PARAM].value);

  if (ifn >= NUM_FNS) {
    ifn = 0;
  }
  
  float (*f)(float, float) = tFns[ifn];
  
  // Compute the sine output
  outputs[AUDIO_OUTPUT].value = clamp(5.0f * f(in, cv), 5.0f);
}


struct DistortionWidget : ModuleWidget {
  DistortionWidget(Distortion *module) : ModuleWidget(module) {
    setPanel(SVG::load(assetPlugin(plugin, "res/Distortion.svg")));

    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(ParamWidget::create<RoundBlackSnapKnob>(Vec(28, 87), module, Distortion::FN_PARAM, 0.0, (1.0 * (NUM_FNS - 1)), 0.0));

    addInput(Port::create<PJ301MPort>(Vec(33, 186), Port::INPUT, module, Distortion::AUDIO_INPUT));

    addInput(Port::create<PJ301MPort>(Vec(18, 220), Port::INPUT, module, Distortion::CV_INPUT));
    addParam(ParamWidget::create<Davies1900hBlackKnob>(Vec(51, 220), module, Distortion::CV_PARAM, -5.0, 5.0, 0.0));
    
    addOutput(Port::create<PJ301MPort>(Vec(33, 275), Port::OUTPUT, module, Distortion::AUDIO_OUTPUT));

    // addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(41, 59), module, Distortion::BLINK_LIGHT));
  }
};


// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelDistortion = Model::create<Distortion, DistortionWidget>("Bargkass", "Distortion", "Distortion module"); //, OSCILLATOR_TAG);
