#include "Bargkass.hpp"
#include <cstdio>

enum Quality {
  Octave,
  Major,
  Minor,
  Major7,
  // Dom7,
  // Minor7,
  // Aug,
  // Dim,
  // Dim7,
  NUM_QUALITIES
};

struct Arpeggiator : Module {
  enum ParamIds {
    // QUALITY_PARAM,
    // INVERSION_PARAM,
    // OUTPUT_LENGTH_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    QUALITY_INPUT,
    VOCT_INPUT,
    CLK_INPUT,
    RST_INPUT,
    // INVERSION_INPUT,
    // OUTPUT_LENGTH_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    // TODO: add lights to display the chosen quality and inversion
    NUM_LIGHTS
  };
  
  // Internal clock
  float elapsed_time = 0.0f;
  
  Arpeggiator() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
  void step() override;

  float root_;
  int note_;
  bool clkLast_;
  Quality quality_ = Octave;
  
  // For more advanced Module features, read Rack's engine.hpp header file
  // - toJson, fromJson: serialization of internal data
  // - onSampleRateChange: event triggered by a change of sample rate
  // - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};

struct sQualDef {
  int len;
  float offsets[4];
};

// Returns the offset in semitones of the nth note in a chord of quality q from the root.
static float chord_offset(Quality q, int n) {
  const struct sQualDef qualities[NUM_QUALITIES] = {
    {1, {0.0f}}, // Octave
    {3, {0.0f, 4.0f, 7.0f}}, // Major
    {3, {0.0f, 3.0f, 7.0f}}, // Minor
    {4, {0.0f, 4.0f, 7.0f, 11.0f}} // Major7
  };

  auto quality = qualities[q];
  
  return quality.offsets[n % quality.len] / 12.0f + (n / quality.len);
}

void Arpeggiator::step() {
  // If no root is fed, don't generate anything.
  if (!inputs[VOCT_INPUT].active) {
    outputs[OUTPUT].value = 0.0f;
    return;
  }
  
  float deltaTime = engineGetSampleTime();
  const int output_length = 6; // TODO: parametrize
  const float duration_per_note = 1.0f; // seconds // TODO: pametrize  
  
  if (!inputs[CLK_INPUT].active) {
    // Accumulate the time to the internal clock
    elapsed_time += deltaTime;
    if (elapsed_time > duration_per_note) {
      elapsed_time = 0.0f;
      note_++;
    }
  } else {
    // Use the external clock. Transitions at >1.7V and <0.1V,
    // triggers the next note on low-to-high transitions.
    if (!clkLast_ && inputs[CLK_INPUT].value > 1.7f) {
      // low to high transition
      clkLast_ = true;
      note_++;
    } else if (clkLast_ && inputs[CLK_INPUT].value < 0.1f){
      // high to low transition
      clkLast_ = false;
    }
  }
  note_ = note_ % output_length;

  if (note_ == 0 || inputs[RST_INPUT].value > 1.7f) {
    root_ = inputs[VOCT_INPUT].value / 5.0f;
    int quality = (int)inputs[QUALITY_INPUT].value;
    if (quality < 0) { quality = 0; }
    if (quality >= NUM_QUALITIES) { quality = NUM_QUALITIES - 1; }
    quality_ =(Quality)quality;
  }
  
  // Compute the output CV in V/oct
  outputs[OUTPUT].value = root_ + chord_offset(quality_, note_);
}


struct ArpeggiatorWidget : ModuleWidget {
  ArpeggiatorWidget(Arpeggiator *module) : ModuleWidget(module) {
    setPanel(SVG::load(assetPlugin(plugin, "res/Arpeggiator.svg")));

    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    // addParam(ParamWidget::create<Davies1900hBlackKnob>(Vec(28, 87), module, Arpeggiator::PITCH_PARAM, -3.0, 3.0, 0.0));

    addInput(Port::create<PJ301MPort>(Vec(60, 186), Port::INPUT, module, Arpeggiator::QUALITY_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(30, 186), Port::INPUT, module, Arpeggiator::VOCT_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(30, 80), Port::INPUT, module, Arpeggiator::CLK_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(60, 80), Port::INPUT, module, Arpeggiator::RST_INPUT));

    addOutput(Port::create<PJ301MPort>(Vec(30, 275), Port::OUTPUT, module, Arpeggiator::OUTPUT));

    // addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(41, 59), module, Arpeggiator::BLINK_LIGHT));
  }
};


// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelArpeggiator = Model::create<Arpeggiator, ArpeggiatorWidget>("Bargkass", "Arpeggiator", "Arpeggiator module"); //, OSCILLATOR_TAG);
