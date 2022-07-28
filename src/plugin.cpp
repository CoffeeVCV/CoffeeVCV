#include "plugin.hpp"

Plugin *pluginInstance;

void init(Plugin *p)
{
	pluginInstance = p;

	// Add modules here
	// p->addModel(modelMyModule);
	p->addModel(modelBetween);
	p->addModel(modelTravel);
	p->addModel(modelHiLo);
	p->addModel(modelSome);
	p->addModel(modelTogether);
	p->addModel(modelTumble);
	p->addModel(modelSet);
	p->addModel(modelFork);
	p->addModel(modelSome2);
	p->addModel(modelTap);
	p->addModel(modelFork2);
	p->addModel(modelSome3);
	p->addModel(modelJuice);
	p->addModel(modelTwinned2);
	p->addModel(modelSet2);
	p->addModel(modelPick);
	p->addModel(modelThese);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
