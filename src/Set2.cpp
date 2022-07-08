#include "plugin.hpp"
#include "components.hpp"
#define NUM_POINTS 20
#define NUM_SETS 4

struct Set2 : Module
{
	enum ParamId
	{
		P_KNOB,
		ENUMS(P_SETBUTTON, NUM_SETS),
		ENUMS(P_TIME, NUM_SETS),
		ENUMS(P_SHAPE, NUM_SETS),
		ENUMS(P_TIMESCALE_SW, NUM_SETS),
		PARAMS_LEN
	};
	enum InputId
	{
		ENUMS(I_SETS, NUM_SETS),
		INPUTS_LEN
	};
	enum OutputId
	{
		O_CV,
		OUTPUTS_LEN
	};
	enum LightId
	{
		ENUMS(L_SET, NUM_SETS),
		ENUMS(L_POINT, NUM_POINTS *2),
		LIGHTS_LEN
	};

	dsp::Timer _setTimer[NUM_SETS];
	dsp::BooleanTrigger _setButtonTrigger[NUM_SETS];
	dsp::SchmittTrigger _setTrigger[NUM_SETS];
	dsp::ClockDivider _lowPriorityClock;
	bool _helddown[NUM_SETS] = {false,false,false,false};
	float _set[NUM_SETS] = {0,0,0,0};

	Set2()
	{
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

			configParam(P_KNOB , 0.f, 10.f, 0.f, "Knob");
			for (int j = 0; j < NUM_SETS; j++)
			{
				configButton(P_SETBUTTON + j, "Long hold to set");
				configInput(I_SETS + j, string::f("Set %d", j + 1));
				configParam(P_TIME + j, 0.f, 1.f, 1.f, string::f("Time %d", j + 1));
				configParam(P_SHAPE + j, 0.f, 1.f, 0.f, string::f("Shape %d", j + 1));
				configSwitch(P_TIMESCALE_SW + j, 0, 2, 0, string::f("Time Scale %d", j + 1),{"1","10","100"});
			}
			configOutput(O_CV, "out");
		
		_lowPriorityClock.setDivision(32);
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
		}
		
		

		// check if button is pressed
		for (int i = 0; i < NUM_SETS; i++)
		{
			bool buttonPressed=_setButtonTrigger[i].process(params[P_SETBUTTON + i].getValue());
			if (buttonPressed)
			{
				if(_helddown[i]==false){
					_setTimer[i].reset();
					_helddown[i]=true;
				}
			}
			if(_helddown[i] && params[P_SETBUTTON + i].getValue()==0){
				if(_setTimer[i].process(args.sampleTime)<1.0){
					params[P_KNOB].setValue(_set[i]);
				}
				_helddown[i]=false;
			}			
			if(_helddown[i] && _setTimer[i].process(args.sampleTime)>1.f){
				_set[i]=params[P_KNOB].getValue();
				lights[L_SET + i].setBrightness(1.f);
				_setTimer[i].reset();
			}
		}
		
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


		y =yOffset + sy*1.5;
		x = mx;
		addParam(createParamCentered<CoffeeKnob30mm>(mm2px(Vec(mx, y)), module, Set2::P_KNOB ));
		// place points in a circle
		float r = 18;
		for (int j = 0; j < NUM_POINTS; j++)
		{
			float range = 360 * 0.75;
			float angle = (((range / NUM_POINTS) * j) + 140) * (M_PI / 180);
			DEBUG("angle: %f, j %d", angle, j);
			float px = x + r * cos(angle);
			float py = y + r * sin(angle);
			addChild(createLightCentered<MediumLight<GreenLight> >(mm2px(Vec(px, py)), module, Set2::L_POINT + j));
		}

		x = xOffset;
		// sets
		for (int j = 0; j < NUM_SETS; j++)
		{
			y = yOffset + sy * 4;
			addChild(createLightCentered<LargeLight<OrangeLight> >(mm2px(Vec(x, y)), module, Set2::L_SET + j));
			y+=sy;
			addParam(createParamCentered<CoffeeInputButton5mm>(mm2px(Vec(x, y)), module, Set2::P_SETBUTTON + j));
			y+=sy;
			addInput(createInputCentered<CoffeeInputPort>(mm2px(Vec(x, y)), module, Set2::I_SETS + j));
			y+=sy;
			addParam(createParamCentered<CoffeeKnob6mm>(mm2px(Vec(x , y)), module, Set2::P_TIME + j));
			y+=sy;
			addParam(createParamCentered<CoffeeKnob6mm>(mm2px(Vec(x , y)), module, Set2::P_SHAPE + j));
			x += sx;
		}

		y += sy * 2;
		addOutput(createOutputCentered<CoffeeOutputPort>(mm2px(Vec(x, y)), module, Set2::O_CV));
		
	}
};

Model *modelSet2 = createModel<Set2, Set2Widget>("Set2");