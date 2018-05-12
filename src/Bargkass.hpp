#include "rack.hpp"

using namespace rack;

// Forward-declare the Plugin, defined in Template.cpp
extern Plugin *plugin;

// Forward-declare each Model, defined in each module source file
extern Model *modelPlayer;
extern Model *modelScope;
extern Model *modelTestDisplay;
extern Model *modelPitchShift;
extern Model *modelOperator;
extern Model *modelAdjuster;
extern Model *modelUnison;

// Forward-declare utils
float clamp(float value, float threshold);
