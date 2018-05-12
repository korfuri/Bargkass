#include "Bargkass.hpp"
#include "TextWidget.hpp"
#include "AudioFile.h"
#include "osdialog.h"
#include <mutex>

class Player : public Module {
public:
  enum ParamIds {
    NUM_PARAMS
  };
  enum InputIds {
    NUM_INPUTS
  };
  enum OutputIds {
    AUDIO_L_OUTPUT,
    AUDIO_R_OUTPUT,
    AUDIO_MONO_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    NUM_LIGHTS
  };
  
};

template<typename Tmodule>
struct AutoUI : ModuleWidget {
  AutoUI(Tmodule *module) : ModuleWidget(module) {
    setPanel(SVG::load(assetPlugin(plugin, "res/AutoUI.svg")));

    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    for (int i = 0; i < Tmodule::NUM_INPUTS; ++i) {
      addInput(Port::create<PJ301MPort>(Vec(10, 10 + 25*i), Port::INPUT, module, i));
    }

    for (int i = 0; i < Tmodule::NUM_OUTPUTS; ++i) {
      addOutput(Port::create<PJ301MPort>(Vec(470, 10 + 25*i), Port::OUTPUT, module, i));
    }

    for (int i = 0; i < Tmodule::NUM_PARAMS; ++i) {
      addParam(ParamWidget::create<RoundBlackSnapKnob>(Vec(50, 10 + 50*i), module, i, -1000.0, +1000.0, 0.0));
    }
  }
};
