#include "Bargkass.hpp"
#include "TextWidget.hpp"
#include <string>
#include <functional>

// A terribly useless module just to test that my TextWidget works the way I want.

struct TestDisplay : Module {
  enum ParamIds {
    NUM_PARAMS
  };
  enum InputIds {
    NUM_INPUTS
  };
  enum OutputIds {
    NUM_OUTPUTS
  };
  enum LightIds {
    NUM_LIGHTS
  };

  TextWidget *time;
  TextWidget *longt;
  TextWidget *vert;
  TextWidget *longvert;
  
  TestDisplay() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
  void step() override;

  float elapsed = 0.0f;
};

void TestDisplay::step() {
  elapsed += engineGetSampleTime();
}


struct TestDisplayWidget : ModuleWidget {
  TestDisplayWidget(TestDisplay *module) : ModuleWidget(module) {
    setPanel(SVG::load(assetPlugin(plugin, "res/TestDisplay.svg")));

    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    TextWidget *time = TextWidget::create<TextWidget>(Rect(Vec(10, 10), Vec(80, 20)), [module]() { return std::to_string(module->elapsed); }, assetPlugin(plugin, "res/VCR_OSD_MONO_1.001.ttf"));
    module->time = time;
    addChild(time);

    TextWidget *longt = TextWidget::create<TextWidget>(Rect(Vec(10, 30), Vec(80, 20)), TextWidget::text("This is a very very very very loooooooooong text"), assetPlugin(plugin, "res/VCR_OSD_MONO_1.001.ttf"));
    module->longt = longt;
    addChild(longt);
    
    TextWidget *vert = TextWidget::create<VerticalTextWidget>(Rect(Vec(10, 300), Vec(230, 20)), TextWidget::text("Can you read this?"), assetPlugin(plugin, "res/VCR_OSD_MONO_1.001.ttf"));
    module->vert = vert;
    addChild(vert);

    TextWidget *longvert = TextWidget::create<VerticalTextWidget>(Rect(Vec(40, 300), Vec(230, 20)), TextWidget::text("This is a very very very very loooooooooong vertical text"), assetPlugin(plugin, "res/VCR_OSD_MONO_1.001.ttf"));
    module->longvert = longvert;
    addChild(longvert);
  }
};

Model *modelTestDisplay = Model::create<TestDisplay, TestDisplayWidget>("Bargkass", "TestDisplay", "TestDisplay module", BLANK_TAG);
