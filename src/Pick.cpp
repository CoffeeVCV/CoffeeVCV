#include "plugin.hpp"
#include "components.hpp"
#define NUM_ROWS 8

struct Pick : Module {
	enum ParamId {
		P_SELECT,
		PARAMS_LEN
	};
	enum InputId {
		I_SELECT,
		I_TRIG,
		ENUMS(I_CV, NUM_ROWS),
		INPUTS_LEN
	};
	enum OutputId {
		O_CV,
		OUTPUTS_LEN
	};
	enum LightId {
		ENUMS(L_SELECT, NUM_ROWS),
		L_SELECT_ERR,
		LIGHTS_LEN
	};

	int _menu_stepInputRange = 1;
	float _selectionRanges[3] = {1.f, 8.f, 10.f};
	float _selectRangeMax=0;
	float _selected=-1;
	dsp::ClockDivider _lowPriorityClock;
	dsp::SchmittTrigger _trigger;
	bool _triggerReady=true;

	Pick() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(P_SELECT, 0.f, NUM_ROWS - 1.f, 0.f, "Select");
		paramQuantities[P_SELECT]->snapEnabled = true;

		configInput(I_TRIG, "Trigger");
		configInput(I_SELECT, "Select CV");
		for(int i=0; i<NUM_ROWS; i++) {
			configInput(I_CV + i, string::f("Input %d", i + 1));
			configLight(L_SELECT + i, string::f("Select %d", i + 1));
		}
		configOutput(O_CV, "Output");
		_lowPriorityClock.setDivision(32);
	}

	void process(const ProcessArgs& args) override {
		//check menus
		_selectRangeMax=_selectionRanges[_menu_stepInputRange];

		//if input is provides use it
		if(inputs[I_SELECT].isConnected()) {
			float v=inputs[I_SELECT].getVoltage();

			//set out of range light
			if(v < 0.f || v > _selectRangeMax) {
				lights[L_SELECT_ERR].setBrightness(1.f);
			} else {
				lights[L_SELECT_ERR].setBrightness(0.f);
			}

			_selected = (int)clamp(v, 0.f, _selectRangeMax);
			DEBUG("selected: %f, range: %f", _selected, _selectRangeMax);
			_selected = (NUM_ROWS+1) * (_selected / _selectRangeMax);
		} else {
			_selected=params[P_SELECT].getValue();
		}

		//check trigger
		bool triggered=_trigger.process(inputs[I_TRIG].getVoltage());
		if(_triggerReady && triggered) {
			_selected++;
			if(_selected >= NUM_ROWS) {
				_selected=0;
			}
			params[P_SELECT].setValue(_selected);
		}
		_triggerReady=!_trigger.isHigh();

		//set lights
		for(int i=0; i<NUM_ROWS; i++) {
			if(_selected>-1)
				lights[L_SELECT + i].setBrightness(_selected==i?1.f:0.f);
		}

		//DEBUG("%f",inputs[I_SELECT].getVoltage());
		outputs[O_CV].setVoltage(inputs[I_CV + (int)_selected].getVoltage());
	
	}
};


struct PickWidget : ModuleWidget {
	PickWidget(Pick* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Pick.svg")));


		float width = 20.32;
		float mx = width / 2;
		float lx = width / 4;
		float rx = width - lx;
		float yOffset = 12;
		float y=yOffset;
		float x=mx;
		float sy=10;
		addInput(createInputCentered<CoffeeInputPortIndicator>(mm2px(Vec(lx, y)), module, Pick::I_SELECT));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(lx+3.5, y + 3.5)), module, Pick::L_SELECT_ERR));
		addInput(createInputCentered<CoffeeInputPort>(mm2px(Vec(rx, y)), module, Pick::I_TRIG));
		y+=sy;
		addParam(createParamCentered<CoffeeKnob10mm>(mm2px(Vec(x, y)), module, Pick::P_SELECT));
		for(int i=0; i<NUM_ROWS; i++){
			y+=sy;
			addInput(createInputCentered<CoffeeInputPort>(mm2px(Vec(lx, y)), module, Pick::I_CV + i));
			addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(rx, y)), module, Pick::L_SELECT + i));
		}
		y+=sy;
		addOutput(createOutputCentered<CoffeeOutputPort>(mm2px(Vec(x, y)), module, Pick::O_CV));
	}

	void appendContextMenu(Menu *menu) override
	{
		Pick *module = dynamic_cast<Pick *>(this->module);
		assert(module);
		menu->addChild(new MenuSeparator());
		menu->addChild(createSubmenuItem("Select Input Scale", "", [=](Menu *menu)
		{
			Menu* ScaleMenu = new Menu();
			ScaleMenu->addChild(createMenuItem("0.0 to 1.0", CHECKMARK(module->_menu_stepInputRange == 0), [module]() { module->_menu_stepInputRange = 0; }));
			ScaleMenu->addChild(createMenuItem("0.0 to 8.0", CHECKMARK(module->_menu_stepInputRange == 1), [module]() { module->_menu_stepInputRange = 1; }));
			ScaleMenu->addChild(createMenuItem("0.0 to 10.0", CHECKMARK(module->_menu_stepInputRange == 2), [module]() { module->_menu_stepInputRange = 2; }));
			menu->addChild(ScaleMenu); 
		}));
	};

};


Model* modelPick = createModel<Pick, PickWidget>("Pick");