#include "Bargkass.hpp"
#include <boost/circular_buffer.hpp>
#include "deps/kissfft/kiss_fft.h"

#include <iostream>

class PitchShift : public Module {
public:
  enum ParamIds {
    SHIFT_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    AUDIO_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    AUDIO_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    NUM_LIGHTS
  };

  static const int TOTAL_BUFFER_LEN = 8192;
  static const int FRAME_LEN = 2048;
  static const int FRAME_INTERVAL = 2048; // 512
  PitchShift() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS),
		 samples_(TOTAL_BUFFER_LEN),
		 samples_out_(TOTAL_BUFFER_LEN),
		 kissCfgFFT_(kiss_fft_alloc(FRAME_LEN, 0, 0, 0)),
		 kissCfgIFFT_(kiss_fft_alloc(FRAME_LEN, 1, 0, 0)) {
    for (auto& f : savedPhases_) {
      f = 0.0f;
    }
  }
  ~PitchShift() {
    free(kissCfgFFT_);
    free(kissCfgIFFT_);
  }

  
  void step() override;

private:
  boost::circular_buffer<float> samples_;
  boost::circular_buffer<float> samples_out_;
  int frames_ = 0;
  kiss_fft_cfg kissCfgFFT_;
  kiss_fft_cfg kissCfgIFFT_;
  float savedPhases_[FRAME_LEN];
};

void PitchShift::step() {
  samples_.push_back(inputs[AUDIO_OUTPUT].value / 5.0f);  
  ++frames_;
  if (frames_ == FRAME_LEN) {
    frames_ -= FRAME_INTERVAL;

    kiss_fft_cpx fftTime[FRAME_LEN], fftFreq[FRAME_LEN];
    auto it = samples_.rbegin();
    for (int n = 0; n < FRAME_LEN; ++n, ++it) {
      fftTime[FRAME_LEN - n].r = *it; // * 0.5 * (1 - cos(2*M_PI*(FRAME_LEN-n) / (FRAME_LEN-1)));
      fftTime[FRAME_LEN - n].i = 0;
    }
    kiss_fft(kissCfgFFT_, fftTime, fftFreq);

    // TEST: shifting
    // int shift = (int)floor(params[SHIFT_PARAM].value);
    // for (int i = shift; i < FRAME_LEN; ++i) {
    //   fftFreq[i - shift].r = fftFreq[i].r;
    //   fftFreq[i - shift].i = fftFreq[i].i;
    // }
    // for (int i = FRAME_LEN - shift; i < FRAME_LEN; ++i) {
    //   fftFreq[i].r = 0.f;
    //   fftFreq[i].i = 0.f;
    // }

    // TEST: bpf
    // for (int i = 0; i < FRAME_LEN; ++i) {
    //   if (!(i > 20 && i < 25)) {
    // 	fftFreq[i].r = 0.f;
    // 	fftFreq[i].i = 0.f;
    //   }
    // }

    // TEST: Align phases?
    float dts = engineGetSampleTime();
    int i = 0;
    for (auto& f : fftFreq) {
      float lastPhase = savedPhases_[i];
      f.i = (f.i + lastPhase);
      if (f.i > M_PI) {
	f.i -= M_PI;
      } else if (f.i < -M_PI) {
	f.i += M_PI;
      }
      savedPhases_[i] = f.i;
      ++i;
      std::cout << f.i << std::endl;
    }

    kiss_fft(kissCfgIFFT_, fftFreq, fftTime);
    for (int i = FRAME_LEN - FRAME_INTERVAL; i != FRAME_LEN; ++i) {
      samples_out_.push_back(fftTime[i].r);
    }
  }
  if (!samples_out_.empty()) {
    outputs[AUDIO_OUTPUT].value = samples_out_.front() * 5.0f / FRAME_LEN;
    samples_out_.pop_front();
  }
}

struct PitchShiftWidget : ModuleWidget {
  PitchShiftWidget(PitchShift *module) : ModuleWidget(module) {
    setPanel(SVG::load(assetPlugin(plugin, "res/TestDisplay.svg")));

    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(ParamWidget::create<RoundBlackSnapKnob>(Vec(28, 87), module, PitchShift::SHIFT_PARAM, 0.0f, 200.0f, 0.0f));
    addInput(Port::create<PJ301MPort>(Vec(33, 186), Port::INPUT, module, PitchShift::AUDIO_INPUT));
    addOutput(Port::create<PJ301MPort>(Vec(60, 235), Port::OUTPUT, module, PitchShift::AUDIO_OUTPUT));
  }
};

Model *modelPitchShift = Model::create<PitchShift, PitchShiftWidget>("Bargkass", "PitchShift", "PitchShift filter", FILTER_TAG);
