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
  
  Player() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
  void step() override;
  json_t *toJson() override;
  void fromJson(json_t *rootJ) override;

  std::string textForDisplay() noexcept;
  void loadSample(std::string path) noexcept;
  
private:
  AudioFile<float> *audioFile_ = nullptr;
  std::string loadedFilePath_ = "";
  std::mutex m_; // guards loadedFilePath_ and audioFile_
  float position_ = 0.0f; // position in the sample, in seconds
};

std::string Player::textForDisplay() noexcept {
  std::lock_guard<std::mutex> lock(m_);
  return loadedFilePath_;
}

json_t *Player::toJson() {
  json_t *rootJ = json_object();
  std::lock_guard<std::mutex> lock(m_);
  json_object_set_new(rootJ, "filePath", json_string(loadedFilePath_.c_str()));
  return rootJ;
}

void Player::fromJson(json_t *rootJ) {
  // Recover the last saved file path
  json_t *filePathJ = json_object_get(rootJ, "filePath");
  if (filePathJ) {
    std::string filePath = json_string_value(filePathJ);
    loadSample(filePath);
  }
}

void Player::loadSample(std::string path) noexcept {
  std::lock_guard<std::mutex> lock(m_);
  if (audioFile_ != nullptr) {
    delete audioFile_;
    audioFile_ = nullptr;
    loadedFilePath_ = "";
  }

  auto try_audioFile = new AudioFile<float>();
  if (try_audioFile->load(path)) {
    audioFile_ = try_audioFile;
    loadedFilePath_ = path;
  } else {
    delete audioFile_;
    loadedFilePath_ = "";
  }
}
  
void Player::step() {  
  std::lock_guard<std::mutex> lock(m_);
  if (audioFile_ == nullptr) {
    return;
  }

  position_ += engineGetSampleTime();
  if (position_ > audioFile_->getLengthInSeconds()) {
    position_ = 0.0f;
  }
  
  int samplePos = floor(position_ * audioFile_->getSampleRate());
  
  outputs[AUDIO_L_OUTPUT].value = 5.0f * audioFile_->samples[0][samplePos];
  if (audioFile_->isStereo()) {
    outputs[AUDIO_R_OUTPUT].value = 5.0f * audioFile_->samples[1][samplePos];
    outputs[AUDIO_MONO_OUTPUT].value = 2.5f * (audioFile_->samples[0][samplePos] + audioFile_->samples[1][samplePos]);
  } else {
    outputs[AUDIO_R_OUTPUT].value = 5.0f * audioFile_->samples[0][samplePos];
    outputs[AUDIO_MONO_OUTPUT].value = 5.0f * audioFile_->samples[0][samplePos];
  }
}


struct PlayerWidget : ModuleWidget {
  PlayerWidget(Player *module) : ModuleWidget(module) {
    setPanel(SVG::load(assetPlugin(plugin, "res/Player.svg")));

    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    // addParam(ParamWidget::create<RoundBlackSnapKnob>(Vec(28, 87), module, Player::FN_PARAM, 0.0, (1.0 * (NUM_FNS - 1)), 0.0));

    // addInput(Port::create<PJ301MPort>(Vec(33, 186), Port::INPUT, module, Player::AUDIO_INPUT));

    // addInput(Port::create<PJ301MPort>(Vec(18, 220), Port::INPUT, module, Player::CV_INPUT));
    // addParam(ParamWidget::create<Davies1900hBlackKnob>(Vec(51, 220), module, Player::CV_PARAM, -5.0, 5.0, 0.0));
    
    addOutput(Port::create<PJ301MPort>(Vec(60, 235), Port::OUTPUT, module, Player::AUDIO_L_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(60, 275), Port::OUTPUT, module, Player::AUDIO_MONO_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(60, 315), Port::OUTPUT, module, Player::AUDIO_R_OUTPUT));
    
    addChild(TextWidget::create<VerticalTextWidget>(Rect(Vec(16, 360), Vec(340.0f, 16.0f)), [module](){return module->textForDisplay();}, assetPlugin(plugin, "res/VCR_OSD_MONO_1.001.ttf"), 12, nvgRGBA(255, 255, 0, 255)));
  }

  Menu *createContextMenu() override;
};


struct LoadFileItem : MenuItem {
  Player *player;

  void onAction(EventAction &e) override {
    char *path = osdialog_file(OSDIALOG_OPEN, ".", NULL, NULL);
    if (path) {
      player->loadSample(path);
      free(path);
    }
  }
};

Menu *PlayerWidget::createContextMenu() {
	Menu *menu = ModuleWidget::createContextMenu();

	MenuLabel *spacerLabel = new MenuLabel();
	menu->addChild(spacerLabel);

	Player *player = dynamic_cast<Player*>(module);
	assert(player);
	
	LoadFileItem *sampleItem = new LoadFileItem();
	sampleItem->text = "Load wav";
	sampleItem->player = player;
	menu->addChild(sampleItem);

	return menu;
}

Model *modelPlayer = Model::create<Player, PlayerWidget>("Bargkass", "Player", "Player module", SAMPLER_TAG);
