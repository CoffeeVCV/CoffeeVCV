#include "plugin.hpp"
#include "components.hpp"
#define NUM_POINTS 40
#define NUM_SETS 4

// TODO [ ] detect manual movement and unset indicator
// TODO [ ] add inverse output
// TODO [ ] change range to 0-1v
// TODO [ ] add menu to set range
// TODO [ ] add offset and scale
// TODO [ ] add input
// TODO [x] json menus for settings
// TODO [ ] add menu item for triggering y/n

struct Set2 : Module
{
	enum ParamId
	{
		P_KNOB,
		ENUMS(P_SETBUTTON, NUM_SETS),
		ENUMS(P_GOBUTTON, NUM_SETS),
		ENUMS(P_TIME, NUM_SETS),
		ENUMS(P_SHAPE, NUM_SETS),
		ENUMS(P_TIMESCALE_SW, NUM_SETS),
		PARAMS_LEN
	};
	enum InputId
	{
		ENUMS(I_GO, NUM_SETS),
		INPUTS_LEN
	};
	enum OutputId
	{
		O_CV,
		ENUMS(O_EOC, NUM_SETS),
		OUTPUTS_LEN
	};
	enum LightId
	{
		ENUMS(L_SET, NUM_SETS),
		ENUMS(L_GO, NUM_SETS * 2),
		ENUMS(L_POINT, NUM_POINTS * 2),
		L_CYCLING,
		LIGHTS_LEN
	};

	dsp::Timer _setTimer[NUM_SETS];
	dsp::BooleanTrigger _setButtonTrigger[NUM_SETS];
	dsp::SchmittTrigger _goTrigger[NUM_SETS];
	dsp::SchmittTrigger _goButtonTrigger[NUM_SETS];
	dsp::ClockDivider _lowPriorityClock;
	dsp::PulseGenerator _eocPulse[NUM_SETS];
	bool _helddown[NUM_SETS] = {false,false,false,false};
	float _set[NUM_SETS] = {0,0,0,0};
	bool _setInUse[NUM_SETS] = {false,false,false,false};
	int _target=-1;
	int _cycleStart=0;
	bool _cycling=false;
	float _shape=0;
	float _cycleProgress=0;
	float _targetDuration=0;
	float _startV=-1;
	float _lastV=-1;
    int _durationScales[3] = {1, 10, 100};
	bool _ready[NUM_SETS]={true,true,true,true};
	bool _retriggerEnabled=false;
	int _lastTarget=-1;
	bool _menuUnifiedEOC=false;
	bool _cycleInterupted=false;



	Set2()
	{
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(P_KNOB , 0.f, 10.f, 0.f, "Knob");
		for (int j = 0; j < NUM_SETS; j++)
		{
			configButton(P_SETBUTTON + j, "Set.  Long hold to clear");
			configButton(P_GOBUTTON + j, "Go");
			configInput(I_GO + j, string::f("Set %d", j + 1));
			configParam(P_TIME + j, 0.f, 1.f, 1.f, string::f("Time %d", j + 1));
			configParam(P_SHAPE + j, -1.f, 1.f, 0.f, string::f("Shape %d", j + 1));
			configSwitch(P_TIMESCALE_SW + j, 0, 2, 0, string::f("Time Scale %d", j + 1),{"1","10","100"});
			configOutput(O_EOC + j, string::f("EOC %d", j + 1));
		}
		configOutput(O_CV, "out");
		_lowPriorityClock.setDivision(32);
	}

	float calculate_shape(float startV, float endV, float p, float shape)
	{
		float exp1 = pow(2, 10 * (p - 1));
		float exp2 = (1 - pow(2, -10 * p));
		float v, v1, v2;
		v1 = startV - ((startV - endV) * p); // linear

		// -1 < p_shape < 1
		if (shape >= 0)
		{
			v2 = startV - ((startV - endV) * exp1);
			v = (v2 * shape) + (v1 * (1 - shape));
		}
		else
		{
			v2 = startV - ((startV - endV) * exp2);
			v = (v2 * abs(shape)) + (v1 * (1 - abs(shape)));
		}
		//DEBUG("calculate_shape: startV: %f, endV: %f, p: %f, shape: %f, v: %f", startV, endV, p, shape,v);
		return v;
	}

	json_t *dataToJson() override
	{
		// save menu items to jason
		json_t *rootJ = json_object();

		json_t *unifiedEOC = json_boolean(_menuUnifiedEOC);
		json_object_set_new(rootJ, "unifiedEOC", unifiedEOC);


		json_t *json_preset = json_array();
		for (int i = 0; i < NUM_SETS; i++)
		{
			json_array_append_new(json_preset, json_real(_set[i]));
			//DEBUG("set %d: %f", i, _set[i]);
		}
		json_object_set_new(rootJ, "Presets", json_preset);
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override
	{
		json_t *unifiedEOC = json_object_get(rootJ, "UnifiedEOC");
		if (unifiedEOC)
		{
			_menuUnifiedEOC = json_boolean_value(unifiedEOC);
		}

		json_t *presetJ = json_object_get(rootJ, "Presets");
		if (presetJ)
		{
			size_t index;
			json_t *value;
			json_array_foreach(presetJ, index, value)
			{
				_set[index] = json_real_value(value);
				if(_set[index]>0)
					_setInUse[index]=true;
				//DEBUG("set %d: %f", index, _set[index]);
			}
		}
	}

	void process(const ProcessArgs &args) override
	{
		if (_lowPriorityClock.process())
		{
			
			//update lights
			float m=paramQuantities[P_KNOB]->getMaxValue();
			int v = (params[P_KNOB].getValue() /  m) * NUM_POINTS;
			for (int j = 0; j < NUM_POINTS; j++)
			{
				lights[L_POINT + j].setBrightness(j < v ? 1.f : 0.f);
			}

			for(int i=0; i<NUM_SETS; i++){
				lights[L_GO+i].setBrightness(_target==i && !_cycleInterupted ?1.f:0.f);
				lights[L_SET+i].setBrightness(_setInUse[i]?1.f:0.f);
			}

			lights[L_CYCLING].setBrightness(_cycling?1.f:0.f);

		}

		// cycle through each set
		for (int i = 0; i < NUM_SETS; i++)
		{
			//check if the set button is pressed
			bool setButtonPressed=_setButtonTrigger[i].process(params[P_SETBUTTON + i].getValue());
			if (setButtonPressed)
			{
				if(_helddown[i]==false){
					_setTimer[i].reset();
					_helddown[i]=true;
				}
			}

			//short hold - set
			if(_helddown[i] && _setInUse[i]==false){
				if(_setTimer[i].process(args.sampleTime)<1.0){
					_setInUse[i]=true;
					_set[i]=params[P_KNOB].getValue();
					lights[L_SET + i].setBrightness(1.f);
				}
				_helddown[i]=false;
			}

			//long hold clear 
			if(_helddown[i] && _setTimer[i].process(args.sampleTime)>1.f){
				_setInUse[i]=false;
				lights[L_SET + i].setBrightness(0.f);
				_helddown[i]=false;
			}

			//check if we need to move towards a value

			//only check for trigger is the set have been saved
			if(_setInUse[i]){
				//make sure the button was released before
				if(_ready[i]){
					//if this set was a retrigger and retriggering enabled
					if(i!=_lastTarget || (i==_lastTarget && _retriggerEnabled)){
						// check button press or trigger
						bool goButtonPressed=_goTrigger[i].process(params[P_GOBUTTON + i].getValue());
						bool goTriggered=_goTrigger[i].process(inputs[I_GO + i].getVoltage());

						if(goButtonPressed || goTriggered){
							_target=i;
							_cycleStart=args.frame;
							_cycling=true;
							_shape=params[P_SHAPE + i].getValue();
							_targetDuration=params[P_TIME + i].getValue() * _durationScales[int(params[P_TIMESCALE_SW + i].getValue())];
							_startV=params[P_KNOB].getValue();
							_lastV=_startV;
							_ready[i]=false;
							_cycleInterupted=false;
						}
					}
				}
				_ready[i]=params[P_GOBUTTON+i].getValue()>0.5 ? false : true;
			}
		}
		
		if (_cycling)
		{
			_cycleProgress = ((args.frame - _cycleStart) / args.sampleRate) / _targetDuration;
			//DEBUG("Cycling %f", _cycleProgress);
			if (_cycleProgress < 1)
			{
				if(_lastV != params[P_KNOB].getValue()){
					_cycling=false;
				}
				
				float v=calculate_shape(_startV, _set[_target], _cycleProgress, _shape);
				params[P_KNOB].setValue(v);
				_lastV=v;
			}
			else
			{ // cycle is finished
				_cycling = false;
				_eocPulse[_target].trigger(1e-3f);
				if(_menuUnifiedEOC){
					outputs[O_EOC+0].setVoltage(10.f);
				} else {
					outputs[O_EOC+_target].setVoltage(10.f);
					//DEBUG("EOC %d", _target);
				}
			}
		}
		//check to clear eoc
		for(int i=0; i<NUM_SETS; i++){
			if(!_eocPulse[i].process(args.sampleTime)){
				if(_menuUnifiedEOC) {
					outputs[O_EOC+0].setVoltage(0.f);			
				}else{
					outputs[O_EOC+i].setVoltage(0.f);
					//DEBUG("EOC off %d", _target);
				}
			}
		}

		//knob was moved, stop the cycle
		if(params[P_KNOB].getValue() != _lastV){
			_cycling=false;
			_cycleInterupted=true;
		}

		//finally output the knob value
		outputs[O_CV].setVoltage(params[P_KNOB].getValue());



	}//process
};//class Set2

struct Set2Widget : ModuleWidget
{
	Set2Widget(Set2 *module)
	{
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Set2.svg")));
		float width = 40.64;
		float mx = width / 2;
		float yOffset = 10;
		float xOffset = 5;
		float sy = 10;
		float sx = 10;
		float x, y;

		// cycling light
		y=yOffset;
		x=xOffset;
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(x, y)), module, Set2::L_CYCLING));

		y =yOffset + sy + 7.55;
		x = mx;
		addParam(createParamCentered<CoffeeKnob30mm>(mm2px(Vec(mx, y)), module, Set2::P_KNOB ));
		// place points in a circle
		float r = 18;
		for (int j = 0; j < NUM_POINTS; j++)
		{
			float range = 360 * 0.75;
			float angle = (((range / NUM_POINTS) * j) + (140)) * (M_PI / 180);
			//DEBUG("angle: %f, j %d", angle, j);
			float px = x + r * cos(angle);
			float py = y + r * sin(angle);
			addChild(createLightCentered<TinyLight<WhiteLight> >(mm2px(Vec(px, py)), module, Set2::L_POINT + j));
		}

		x = xOffset;
		// sets
		for (int j = 0; j < NUM_SETS; j++)
		{
			y = yOffset + (sy * 3) + 7.5;
			addChild(createLightCentered<LargeLight<GreenLight> >(mm2px(Vec(x, y)), module, Set2::L_GO + j));
			y+=sy;
			addParam(createParamCentered<CoffeeInput5mmButtonButtonIndicator>(mm2px(Vec(x, y)), module, Set2::P_GOBUTTON + j));
			addParam(createParamCentered<CoffeeTinyButton>(mm2px(Vec(x+3.5, y-3.5)), module, Set2::P_SETBUTTON + j));
			addChild(createLightCentered<SmallSimpleLight<OrangeLight> >(mm2px(Vec(x+3.5, y+3.5)), module, Set2::L_SET + j));
			y+=sy;
			addInput(createInputCentered<CoffeeInputPort>(mm2px(Vec(x, y)), module, Set2::I_GO + j));
			y+=sy-2.5;
			addParam(createParamCentered<CoffeeSwitch3PosHori>(mm2px(Vec(x, y)), module, Set2::P_TIMESCALE_SW + j));
			y+=sy-2.5;
			addParam(createParamCentered<CoffeeKnob6mm>(mm2px(Vec(x , y)), module, Set2::P_TIME + j));
			y+=sy-2.5;
			addParam(createParamCentered<CoffeeKnob6mm>(mm2px(Vec(x , y)), module, Set2::P_SHAPE + j));
			y+=sy;
			addOutput(createOutputCentered<CoffeeOutputPort>(mm2px(Vec(x, y)), module, Set2::O_EOC + j));
			x+=sx;
		}
		y += sy ;
		x = mx;
		addOutput(createOutputCentered<CoffeeOutputPort>(mm2px(Vec(x, y)), module, Set2::O_CV));
	}
	void appendContextMenu(Menu* menu) override {
		Set2* module = dynamic_cast<Set2*>(this->module);
		assert(module);
		menu->addChild(new MenuSeparator());
		menu->addChild(createSubmenuItem("End Of Cycle", "", [=](Menu* menu) {
		Menu* EOCMenu = new Menu();
		EOCMenu->addChild(createMenuItem("Single End of Cycle", CHECKMARK(module->_menuUnifiedEOC == true), [module]() { module->_menuUnifiedEOC = !module->_menuUnifiedEOC; }));
		menu->addChild(EOCMenu);
		}));
	};
};

Model *modelSet2 = createModel<Set2, Set2Widget>("Set2");