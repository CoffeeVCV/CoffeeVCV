#include "plugin.hpp"
#include "components.hpp"
#define NUM_ROWS 8
#define NUM_SLOTS 16

// [ ] Todo - fix 1st load, seem unintuiative
// [ ] TODO - update to/from json functions

struct Juice : Module
{
	enum ParamId
	{
		P_SAVEBUTTON,
		P_LOADBUTTON,
		P_NEXTBUTTON,
		P_PREVBUTTON,
		P_NEXTACTIVEBUTTON,
		P_PREVACTIVEBUTTON,
		P_RANDOMACTIVEBUTTON,
		ENUMS(P_V, NUM_ROWS),
		PARAMS_LEN
	};
	enum InputId
	{
		I_SELECT,
		I_RANDOMACTIVE,
		I_PREVACTIVE,
		I_NEXTACTIVE,
		INPUTS_LEN
	};
	enum OutputId
	{
		ENUMS(O_CV1, NUM_ROWS),
		OUTPUTS_LEN
	};
	enum LightId
	{
		L_SLOTINUSE,
		LIGHTS_LEN
	};

	struct slot
	{
		bool active = false;
		float v[NUM_ROWS] = {0.f};
	};

	struct presetControl
	{
		slot preset[NUM_SLOTS] = {};
		int currentSlot = 0;
		bool anyUsedSlots = false;
		bool changed = false;

		void nextSlot()
		{
			currentSlot = (currentSlot+1) % NUM_SLOTS;
		}

		void prevSlot()
		{
			currentSlot = (--currentSlot < 0) ? NUM_SLOTS - 1 : currentSlot;
		}
		void saveSlot(std::vector<Param> &p)
		{
			for (int i = 0; i < NUM_ROWS; i++)
			{
				preset[currentSlot].v[i] = p[P_V + i].getValue();
			}
			preset[currentSlot].active = true;
			anyUsedSlots = true;
		}

		void loadSlot(std::vector<Param> &p)
		{
			for (int i = 0; i < NUM_ROWS; i++)
			{
				p[P_V + i].setValue(preset[currentSlot].v[i]);
				changed = true;
			}
		}

		void prevActiveSlot()
		{
			if (anyUsedSlots)
			{
				int i = currentSlot;
				bool found = false;
				while (!found)
				{
					i = (--i < 0) ? NUM_SLOTS - 1 : i;
					found = preset[i].active;
				}
				currentSlot = i;
			}
		}

		void nextActiveSlot()
		{
			if (anyUsedSlots)
			{
				bool found = false;
				int i = currentSlot;
				while (!found)
				{
					i = (i+1) % NUM_SLOTS;
					found = preset[i].active;
				}
				currentSlot = i;
			}
		}
	};

	dsp::ClockDivider _lowPriorityDivider;
	dsp::BooleanTrigger _saveButtonTrigger;
	dsp::BooleanTrigger _loadButtonTrigger;
	dsp::BooleanTrigger _nextButtonTrigger;
	dsp::BooleanTrigger _prevButtonTrigger;
	dsp::BooleanTrigger _nextActiveButtonTrigger;
	dsp::BooleanTrigger _prevActiveButtonTrigger;
	dsp::BooleanTrigger _randomActiveButtonTrigger;
	dsp::SchmittTrigger _nextActiveTrigger;
	dsp::SchmittTrigger _prevActiveTrigger;
	dsp::SchmittTrigger _randomActiveTrigger;
	presetControl _presetControl;
	bool _prevTriggerReady = true;
	bool _nextTriggerReady = true;
	bool _randomTriggerReady = true;
	bool _polyphonic = false;

	Juice()
	{
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configButton(P_SAVEBUTTON, "Save");
		configButton(P_LOADBUTTON, "Load");
		configButton(P_NEXTBUTTON, "Next");
		configButton(P_PREVBUTTON, "Prev");
		configButton(P_NEXTACTIVEBUTTON, "Next Active");
		configButton(P_PREVACTIVEBUTTON, "Prev Active");
		for (int i = 0; i < NUM_ROWS; i++)
		{
			configParam(P_V + i, -10, 10, 0, "V" + string::f("Setter %d", i + 1));
			configOutput(O_CV1 + i, "CV" + string::f("CV %d", i + 1));
		}
		configInput(I_SELECT, "Select");
		configInput(I_RANDOMACTIVE, "Random Select Active");
		configInput(I_PREVACTIVE, "Prev Active");
		configInput(I_NEXTACTIVE, "Next Active");
		_lowPriorityDivider.setDivision(16);
	}

	void onRandomize() override
	{
		_presetControl.changed = true;
	}

	json_t *dataToJson() override
	{
		// iterate through the presets struct and return as a json structure
		json_t *rootJ = json_object();
		for (int n = 0; n < NUM_SLOTS; n++)
		{
			json_t *json_preset = json_array();
			if (_presetControl.preset[n].active)
			{
				for (int i = 0; i < NUM_ROWS; i++)
				{
					json_array_append_new(json_preset, json_real(_presetControl.preset[n].v[i]));
				}
				std::string name = string::f("PresetSlot%d", n);
				json_object_set_new(rootJ, name.c_str(), json_preset);
			}
		}
		json_t *polyphony = json_boolean(_polyphonic);
		json_object_set_new(rootJ, "Polyphonic", polyphony);
		json_t *currentSlot = json_integer(_presetControl.currentSlot);
		json_object_set_new(rootJ, "CurrentSlot", currentSlot);
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override
	{
		for (int n = 0; n < NUM_SLOTS; n++)
		{
			std::string name = string::f("PresetSlot%d", n);
			json_t *presetJ = json_object_get(rootJ, name.c_str());
			if (presetJ)
			{
				size_t index;
				json_t *value;
				json_array_foreach(presetJ, index, value)
				{
					_presetControl.preset[n].v[index] = json_real_value(value);
				}
				_presetControl.preset[n].active = true;
				_presetControl.anyUsedSlots = true;
			}
			else
			{
				_presetControl.preset[n].active = false;
			}
		}
		json_t *polyphony = json_object_get(rootJ, "Polyphonic");
		if (polyphony)
		{
			_polyphonic = json_boolean_value(polyphony);
		}

		json_t *currentSlot = json_object_get(rootJ, "CurrentSlot");
		if (currentSlot)
		{
			_presetControl.currentSlot = json_integer_value(currentSlot);
			if (_presetControl.currentSlot < 0)
			{
				_presetControl.currentSlot = 0;
			}
			else if (_presetControl.currentSlot >= NUM_SLOTS)
			{
				_presetControl.currentSlot = NUM_SLOTS - 1;
			}
			_presetControl.loadSlot(params);
		}
	}

	// void output()
	// {
	// 	if (_polyphonic)
	// 	{
	// 		outputs[O_CV1 + 0].setChannels(NUM_ROWS);
	// 	} else {
	// 		outputs[O_CV1 + 0].setChannels(1);
	// 	}
	// 	for (int i = 0; i < NUM_ROWS; i++)
	// 	{
	// 		if (_polyphonic)
	// 		{
	// 			outputs[O_CV1 + 0].setVoltage(params[P_V + i].getValue(), i);
	// 		}
	// 		else
	// 		{
	// 			outputs[O_CV1 + i].setVoltage(params[P_V + i].getValue());
	// 		}
	// 	}
	// }

	void process(const ProcessArgs &args) override
	{
		// check if input select is being used
		if (inputs[I_SELECT].isConnected())
		{
			float selectVoltage = inputs[I_SELECT].getVoltage();
			// convert voltage to selection
			int selection = clamp(int(selectVoltage), 1, NUM_SLOTS) - 1;
			if (selection != _presetControl.currentSlot)
			{
				_presetControl.currentSlot = selection;
				_presetControl.loadSlot(params);
			}
		}

		// check if input random active is being triggers
		bool doRandom = true;
		doRandom &= _randomTriggerReady;
		doRandom &= _randomActiveTrigger.process(inputs[I_RANDOMACTIVE].getVoltage());
		doRandom |= _randomActiveButtonTrigger.process(params[P_RANDOMACTIVEBUTTON].getValue());
		if (doRandom)
		{
			// just cointoss up or down
			if (random::uniform() > 0.5)
			{
				_presetControl.nextActiveSlot();
			}
			else
			{
				_presetControl.prevActiveSlot();
			}
			_presetControl.loadSlot(params);
		}
		_randomTriggerReady = !_randomActiveTrigger.isHigh();

		// select next active slot
		if (_nextTriggerReady & _nextActiveTrigger.process(inputs[I_NEXTACTIVE].getVoltage()))
		{
			_presetControl.nextActiveSlot();
			_presetControl.loadSlot(params);
		}
		_nextTriggerReady = (!_nextActiveTrigger.isHigh());

		// select prev active slot
		if (_prevTriggerReady & _prevActiveTrigger.process(inputs[I_PREVACTIVE].getVoltage()))
		{
			_presetControl.prevActiveSlot();
			_presetControl.loadSlot(params);
		}
		_prevTriggerReady = (!_prevActiveTrigger.isHigh());

		if (_lowPriorityDivider.process())
		{
			if (_saveButtonTrigger.process(params[P_SAVEBUTTON].getValue()))
			{
				_presetControl.saveSlot(params);
			}

			if (_loadButtonTrigger.process(params[P_LOADBUTTON].getValue()))
			{
				_presetControl.loadSlot(params);
			}

			if (_nextButtonTrigger.process(params[P_NEXTBUTTON].getValue()))
			{
				_presetControl.nextSlot();
			}

			if (_prevButtonTrigger.process(params[P_PREVBUTTON].getValue()))
			{
				_presetControl.prevSlot();
			}
			if (_nextActiveButtonTrigger.process(params[P_NEXTACTIVEBUTTON].getValue()))
			{
				_presetControl.nextActiveSlot();
				_presetControl.loadSlot(params);
			}

			if (_prevActiveButtonTrigger.process(params[P_PREVACTIVEBUTTON].getValue()))
			{
				_presetControl.prevActiveSlot();
				_presetControl.loadSlot(params);
			}

			lights[L_SLOTINUSE].value = _presetControl.preset[_presetControl.currentSlot].active;
		}// end low priority divider

		//set polyphony channels
		outputs[O_CV1 + 0].setChannels(_polyphonic ? NUM_ROWS : 1);

		for (int i = 0; i < NUM_ROWS; i++)
		{
			if (_polyphonic)
			{
				outputs[O_CV1 + 0].setVoltage(params[P_V + i].getValue(), i);
			}
			else
			{
				outputs[O_CV1 + i].setVoltage(params[P_V + i].getValue(), 0);
			}
		}

	}
};

struct DigitalDisplay : Widget
{
	std::string fontPath;
	std::string bgText;
	std::string text;
	float fontSize;
	NVGcolor bgColor = nvgRGB(0x46, 0x46, 0x46);
	NVGcolor fgColor = SCHEME_ORANGE;
	Vec textPos;

	void prepareFont(const DrawArgs &args)
	{
		// Get font
		std::shared_ptr<Font> font = APP->window->loadFont(fontPath);
		if (!font)
			return;
		nvgFontFaceId(args.vg, font->handle);
		nvgFontSize(args.vg, fontSize);
		nvgTextLetterSpacing(args.vg, 0.0);
		nvgTextAlign(args.vg, NVG_ALIGN_RIGHT);
	}

	void draw(const DrawArgs &args) override
	{
		// Background
		nvgBeginPath(args.vg);
		nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, 2);
		nvgFillColor(args.vg, nvgRGB(0x1C, 0x0A, 0x00)); // 1c0a00
		nvgFill(args.vg);

		prepareFont(args);

		// Background text
		nvgFillColor(args.vg, bgColor);
		nvgText(args.vg, textPos.x, textPos.y, bgText.c_str(), NULL);
	}

	void drawLayer(const DrawArgs &args, int layer) override
	{
		if (layer == 1)
		{
			prepareFont(args);
			// Foreground text
			nvgFillColor(args.vg, fgColor);
			nvgText(args.vg, textPos.x, textPos.y, text.c_str(), NULL);
		}
		Widget::drawLayer(args, layer);
	}
};

struct NinetyNineDisplay : DigitalDisplay
{
	NinetyNineDisplay()
	{
		fontPath = asset::system("res/fonts/DSEG7ClassicMini-BoldItalic.ttf");
		textPos = Vec(27.5, 20);
		bgText = "88";
		fontSize = 16;
	}
};

struct PresetDisplay : NinetyNineDisplay
{
	Juice *module;
	void step() override
	{
		int slots = NUM_SLOTS;
		if (module)
		{
			slots = module->_presetControl.currentSlot + 1;
		}
		text = string::f("%d", slots);
	}
};

struct JuiceWidget : ModuleWidget
{
	JuiceWidget(Juice *module)
	{
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Juice.svg")));

		float sx = 10;
		float sy = 10;
		float yOffset = 12;
		float xOffset = 5;

		float x = xOffset;
		float y = yOffset;
		// save load next prev

		x = xOffset + sx;
		addParam(createParamCentered<Coffee3mmButton>(mm2px(Vec(x - 1.5, y - 2)), module, Juice::P_SAVEBUTTON));
		addParam(createParamCentered<Coffee3mmButton>(mm2px(Vec(x + 2.5, y - 2)), module, Juice::P_LOADBUTTON));
		addParam(createParamCentered<Coffee3mmButton>(mm2px(Vec(x - 1.5, y + 2)), module, Juice::P_PREVBUTTON));
		addParam(createParamCentered<Coffee3mmButton>(mm2px(Vec(x + 2.5, y + 2)), module, Juice::P_NEXTBUTTON));
		addChild(createLightCentered<TinyLight<GreenLight> >(mm2px(Vec(x + 0.5, y)), module, Juice::L_SLOTINUSE));

		y = yOffset + sy;
		x = xOffset;
		addInput(createInputCentered<CoffeeInputPortButton>(mm2px(Vec(x, y)), module, Juice::I_PREVACTIVE));
		addParam(createParamCentered<CoffeeTinyButton>(mm2px(Vec(x + 3.5, y - 3.5)), module, Juice::P_PREVACTIVEBUTTON));
		addInput(createInputCentered<CoffeeInputPortButton>(mm2px(Vec(x + sx, y)), module, Juice::I_NEXTACTIVE));
		addParam(createParamCentered<CoffeeTinyButton>(mm2px(Vec(x + sx + 3.5, y - 3.5)), module, Juice::P_NEXTACTIVEBUTTON));

		y = yOffset + sy + sy;
		x = xOffset;
		addInput(createInputCentered<CoffeeInputPort>(mm2px(Vec(x, y)), module, Juice::I_SELECT));
		addInput(createInputCentered<CoffeeInputPortButton>(mm2px(Vec(x + sx, y)), module, Juice::I_RANDOMACTIVE));
		addParam(createParamCentered<CoffeeTinyButton>(mm2px(Vec(x + sx + 3.5, y - 3.5)), module, Juice::P_RANDOMACTIVEBUTTON));

		x = xOffset;
		for (int i = 0; i < NUM_ROWS; i++)
		{
			y = yOffset + sy + sy + sy + (i * sy);
			addParam(createParamCentered<CoffeeKnob6mm>(mm2px(Vec(x, y)), module, Juice::P_V + i));
			addOutput(createOutputCentered<CoffeeOutputPort>(mm2px(Vec(x + sx, y)), module, Juice::O_CV1 + i));
		}

		x = 1;
		y = yOffset;
		PresetDisplay *display = createWidget<PresetDisplay>(mm2px(Vec(x, yOffset - 4)));
		display->box.size = mm2px(Vec(10, 8));
		display->module = module;
		addChild(display);
	}

	void appendContextMenu(Menu *menu) override
	{
		Juice *module = dynamic_cast<Juice *>(this->module);
		assert(module);
		menu->addChild(new MenuSeparator());
		menu->addChild(createSubmenuItem("Polyphonic", "", [=](Menu *menu)
										 {
			Menu* polyphonicMenu = new Menu();
			polyphonicMenu->addChild(createMenuItem("Polyphonic", CHECKMARK(module->_polyphonic == true), [module]() { module->_polyphonic = true; }));
			polyphonicMenu->addChild(createMenuItem("Monophonic", CHECKMARK(module->_polyphonic == false), [module]() { module->_polyphonic = false; }));
			menu->addChild(polyphonicMenu); }));
	};
};

Model *modelJuice = createModel<Juice, JuiceWidget>("Juice");