#include "Bargkass.hpp"
#include "Chords.hpp"
#include "CallbackButton.hpp"
#include "TextWidget.hpp"
#include "dsp/digital.hpp"
#include <vector>
#include <mutex>

// TODO remove
#include <iostream>

class ChordProg : public Module, public TextOutputsModule {
public:
  enum ParamIds {
    NUM_PARAMS
  };
  enum InputIds {
    CLOCK_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    NOTE1_OUT,
    NOTE2_OUT,
    NOTE3_OUT,
    NOTE4_OUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    L1R, L1G, L1B,
    L2R, L2G, L2B,
    L3R, L3G, L3B,
    L4R, L4G, L4B,
    L5R, L5G, L5B,
    L6R, L6G, L6B,
    L7R, L7G, L7B,
    L8R, L8G, L8B,
    L9R, L9G, L9B,
    L10R, L10G, L10B,
    L11R, L11G, L11B,
    L12R, L12G, L12B,
    L13R, L13G, L13B,
    L14R, L14G, L14B,
    L15R, L15G, L15B,
    L16R, L16G, L16B,
    NUM_LIGHTS
  };
  enum TextIds {
    CHORD_TEXT1,
    CHORD_TEXT2,
    CHORD_TEXT3,
    CHORD_TEXT4,
    CHORD_TEXT5,
    CHORD_TEXT6,
    CHORD_TEXT7,
    CHORD_TEXT8,
    CHORD_TEXT9,
    CHORD_TEXT10,
    CHORD_TEXT11,
    CHORD_TEXT12,
    CHORD_TEXT13,
    CHORD_TEXT14,
    CHORD_TEXT15,
    CHORD_TEXT16,
    NUM_TEXTS
  };
  
  static constexpr const float C4 = 261.626f;
  
  ChordProg() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS),
		TextOutputsModule(NUM_TEXTS) {
    chords_.push_back(Chord("C", 0.0f));
  }
  ~ChordProg() {}
  
  void step() override;
  json_t *toJson() override;
  void fromJson(json_t *rootJ) override;
  
  void advancePlay();
  void advanceEdit();
  void addChord();
  void deleteChord();

  void shift_third();
  void shift_fifth();
  void shift_seventh();
  void makeMajor();
  void makeMinor();
  void makeSeventh();
  void setRoot(std::string, float);
  
  std::string describe();
private:
  void setLight(int i, int r, int g, int b);

  std::vector<Chord> chords_;
  unsigned int playidx_ = 0;
  unsigned int editidx_ = 0;
  std::mutex chords_m_;  // guards chords_ and idx_
  SchmittTrigger clockInTrigger_;  // Schmitt trigger for CLOCK_INPUT
};

void ChordProg::step() {
  if (clockInTrigger_.process(rescale(inputs[CLOCK_INPUT].value, 0.1f, 1.7f, 0.f, 1.f))) {
    advancePlay();
  }

  std::lock_guard<std::mutex> l(chords_m_);
  //text_outputs[CURRENT_CHORD_TEXT].value = chords_[editidx_].describe();
  for (unsigned int i = 0; i < 16; ++i) {
    if (i >= chords_.size()) {
      setLight(i, 0, 0, 0);
      text_outputs[CHORD_TEXT1 + i].value = "";
      continue;
    }
    int r = 0, g = 0, b = 60;
    if (i == editidx_) { r = 255; b = 0; }
    if (i == playidx_) { g = 255; b = 0; }
    setLight(i, r, g, b);
    text_outputs[CHORD_TEXT1 + i].value = chords_[i].describe();
  }
  outputs[NOTE1_OUT].value = chords_[playidx_].out(0);
  outputs[NOTE2_OUT].value = chords_[playidx_].out(1);
  outputs[NOTE3_OUT].value = chords_[playidx_].out(2);
  outputs[NOTE4_OUT].value = chords_[playidx_].out(3);
}

json_t* ChordProg::toJson() {
  std::lock_guard<std::mutex> l(chords_m_);
  json_t *rootJ = json_object();
  json_t *chords = json_array();
  for (unsigned int i = 0; i < chords_.size(); i++) {
    json_t *c = chords_[i].toJson(); 
    json_array_append_new(chords, c);
  }
  json_object_set_new(rootJ, "chords", chords);
  return rootJ;
}

void ChordProg::fromJson(json_t* rootJ) {
  json_t* chordsJ = json_object_get(rootJ, "chords");
  if (chordsJ == nullptr)
    return;

  size_t i;
  json_t *value;
  std::lock_guard<std::mutex> l(chords_m_);
  chords_.clear();
  
  json_array_foreach(chordsJ, i, value) {
    if (i >= 16) break;
    Chord c;
    c.fromJson(value);
    chords_.push_back(c);
  }
}

void ChordProg::setLight(int i, int r, int g, int b) {
  lights[L1R + 3 * i].value = (float)r / 255.f;
  lights[L1G + 3 * i].value = (float)g / 255.f;
  lights[L1B + 3 * i].value = (float)b / 255.f;
}

void ChordProg::advancePlay() {
  std::lock_guard<std::mutex> l(chords_m_);
  playidx_ = (playidx_ + 1) % chords_.size();
}

void ChordProg::advanceEdit() {
  std::lock_guard<std::mutex> l(chords_m_);
  editidx_ = (editidx_ + 1) % chords_.size();
}

void ChordProg::addChord() {
  std::lock_guard<std::mutex> l(chords_m_);
  if (chords_.size() == 16)
    return;
  chords_.push_back(chords_[editidx_]);
  ++editidx_;
}

void ChordProg::deleteChord() {
  std::lock_guard<std::mutex> l(chords_m_);
  if (chords_.size() == 1)
    return;
  chords_.erase(chords_.begin() + editidx_);
  if (editidx_ >= chords_.size()) {
    editidx_ = chords_.size() - 1;
  }
  if (playidx_ >= chords_.size()) {
    playidx_ = chords_.size() - 1;
  }
}

void ChordProg::makeMajor() {
  std::lock_guard<std::mutex> l(chords_m_);
  chords_[editidx_].make_major();
}

void ChordProg::makeMinor() {
  std::lock_guard<std::mutex> l(chords_m_);
  chords_[editidx_].make_minor();
}

void ChordProg::makeSeventh() {
  std::lock_guard<std::mutex> l(chords_m_);
  chords_[editidx_].make_seven();
}

void ChordProg::shift_third() {
  std::lock_guard<std::mutex> l(chords_m_);
  chords_[editidx_].shift_third();
}

void ChordProg::shift_fifth() {
  std::lock_guard<std::mutex> l(chords_m_);
  chords_[editidx_].shift_fifth();
}

void ChordProg::shift_seventh() {
  std::lock_guard<std::mutex> l(chords_m_);
  chords_[editidx_].shift_seventh();
}

void ChordProg::setRoot(std::string name, float voct) {
  std::lock_guard<std::mutex> l(chords_m_);
  chords_[editidx_].set_root(name, voct);
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

#define COL_INPUTS_A 60
#define COL_ADD_BUTTON 100
#define COL_DEL_BUTTON 140
#define COL_NXT_BUTTON 180

#define HPAD 30
#define HLINE 20
#define COL_LIGHTS 20
#define COL_TEXTS 30

#define LINE_BUTTONS_NAV 90
#define LINE_BUTTONS_SHORTCUTS 130
#define LINE_BUTTONS_ROOTS_BLACK 170
#define LINE_BUTTONS_ROOTS_WHITE 190
#define LINE_CLOCK 30
#define LINE_OUTS 350

// Half diameter of round objects
#define HPORTSIZE 12
#define HHUGEKNOBSIZE 30
#define HKNOBSIZE 15
#define HATTVSIZE 11
#define HSWITCHSIZE 12
#define HLEDSIZE 6

struct ChordProgWidget : ModuleWidget {
  ChordProgWidget(ChordProg *module) : ModuleWidget(module) {
    setPanel(SVG::load(assetPlugin(plugin, "res/ChordProg.svg")));

    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addInput(Port::create<PJ301MPort>(Vec(COL_INPUTS_A - HPORTSIZE, LINE_CLOCK - HPORTSIZE), Port::INPUT, module, ChordProg::CLOCK_INPUT));

    for (int n = 0; n < 16; ++n) {
      float line = HPAD + (box.size.y - HPAD) / 16.f * n;
      addChild(ModuleLightWidget::create<SmallLight<RedGreenBlueLight>>(Vec(COL_LIGHTS, line - HLEDSIZE), module, ChordProg::L1R + 3 * n));
      addChild(TextWidget::create<TextWidget>(Rect(Vec(COL_TEXTS, line - 10), Vec(50, 20)), [module, n](){ return module->text_outputs[ChordProg::CHORD_TEXT1 + n].value; }, assetPlugin(plugin, "res/VCR_OSD_MONO_1.001.ttf"), 12, nvgRGBA(255, 255, 0, 255)));
    } 
        
    addChild(CallbackButton::create(Vec(COL_ADD_BUTTON, LINE_BUTTONS_NAV), Vec(30, 30), "+", [module](){ module->addChord(); }));
    addChild(CallbackButton::create(Vec(COL_DEL_BUTTON, LINE_BUTTONS_NAV), Vec(30, 30), "-", [module](){ module->deleteChord(); }));
    addChild(CallbackButton::create(Vec(COL_NXT_BUTTON, LINE_BUTTONS_NAV), Vec(30, 30), ">", [module](){ module->advanceEdit(); }));

    addChild(CallbackButton::create(Vec(COL_ADD_BUTTON, LINE_BUTTONS_SHORTCUTS), Vec(30, 30), "M", [module](){ module->makeMajor(); }));
    addChild(CallbackButton::create(Vec(COL_DEL_BUTTON, LINE_BUTTONS_SHORTCUTS), Vec(30, 30), "m", [module](){ module->makeMinor(); }));
    addChild(CallbackButton::create(Vec(COL_NXT_BUTTON, LINE_BUTTONS_SHORTCUTS), Vec(30, 30), "7", [module](){ module->makeSeventh(); }));

    addChild(CallbackButton::create(Vec(90.f + (box.size.x - 10.f) / 12.f * 1, LINE_BUTTONS_ROOTS_WHITE), Vec(25, 20), " ", [module](){ module->setRoot("C", 0 / 12.f); }));
    addChild(CallbackButton::create(Vec(90.f + (box.size.x - 10.f) / 12.f * 2, LINE_BUTTONS_ROOTS_WHITE), Vec(25, 20), " ", [module](){ module->setRoot("D", 2 / 12.f); }));
    addChild(CallbackButton::create(Vec(90.f + (box.size.x - 10.f) / 12.f * 3, LINE_BUTTONS_ROOTS_WHITE), Vec(25, 20), " ", [module](){ module->setRoot("E", 4 / 12.f); }));
    addChild(CallbackButton::create(Vec(90.f + (box.size.x - 10.f) / 12.f * 4, LINE_BUTTONS_ROOTS_WHITE), Vec(25, 20), " ", [module](){ module->setRoot("F", 5 / 12.f); }));
    addChild(CallbackButton::create(Vec(90.f + (box.size.x - 10.f) / 12.f * 5, LINE_BUTTONS_ROOTS_WHITE), Vec(25, 20), " ", [module](){ module->setRoot("G", 7 / 12.f); }));
    addChild(CallbackButton::create(Vec(90.f + (box.size.x - 10.f) / 12.f * 6, LINE_BUTTONS_ROOTS_WHITE), Vec(25, 20), " ", [module](){ module->setRoot("A", 9 / 12.f); }));
    addChild(CallbackButton::create(Vec(90.f + (box.size.x - 10.f) / 12.f * 7, LINE_BUTTONS_ROOTS_WHITE), Vec(25, 20), " ", [module](){ module->setRoot("B", 11 / 12.f); }));
    addChild(CallbackButton::create(Vec(102.f + (box.size.x - 10.f) / 12.f * 1, LINE_BUTTONS_ROOTS_BLACK), Vec(25, 20), " ", [module](){ module->setRoot("C#", 1 / 12.f); }));
    addChild(CallbackButton::create(Vec(102.f + (box.size.x - 10.f) / 12.f * 2, LINE_BUTTONS_ROOTS_BLACK), Vec(25, 20), " ", [module](){ module->setRoot("D#", 3 / 12.f); }));
    addChild(CallbackButton::create(Vec(102.f + (box.size.x - 10.f) / 12.f * 4, LINE_BUTTONS_ROOTS_BLACK), Vec(25, 20), " ", [module](){ module->setRoot("F#", 6 / 12.f); }));
    addChild(CallbackButton::create(Vec(102.f + (box.size.x - 10.f) / 12.f * 5, LINE_BUTTONS_ROOTS_BLACK), Vec(25, 20), " ", [module](){ module->setRoot("G#", 8 / 12.f); }));
    addChild(CallbackButton::create(Vec(102.f + (box.size.x - 10.f) / 12.f * 6, LINE_BUTTONS_ROOTS_BLACK), Vec(25, 20), " ", [module](){ module->setRoot("A#", 10 / 12.f); }));
    
    addOutput(Port::create<PJ301MPort>(Vec(100, LINE_OUTS), Port::OUTPUT, module, ChordProg::NOTE1_OUT));
    addOutput(Port::create<PJ301MPort>(Vec(130, LINE_OUTS), Port::OUTPUT, module, ChordProg::NOTE2_OUT));
    addOutput(Port::create<PJ301MPort>(Vec(160, LINE_OUTS), Port::OUTPUT, module, ChordProg::NOTE3_OUT));
    addOutput(Port::create<PJ301MPort>(Vec(190, LINE_OUTS), Port::OUTPUT, module, ChordProg::NOTE4_OUT));
  }
};

Model *modelChordProg = Model::create<ChordProg, ChordProgWidget>("Bargkass", "ChordProg", "Chord Progression", OSCILLATOR_TAG, QUAD_TAG);
