#include "Bargkass.hpp"


Plugin *plugin;


void init(Plugin *p) {
	plugin = p;
	p->slug = TOSTRING(SLUG);
	p->version = TOSTRING(VERSION);

	// Add all Models defined throughout the plugin
	p->addModel(modelPlayer);
	p->addModel(modelTestDisplay);
	p->addModel(modelScope);
	p->addModel(modelPitchShift);
	p->addModel(modelOperator);
	p->addModel(modelAdjuster);
	p->addModel(modelUnison);
	
	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
