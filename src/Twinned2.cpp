#include "plugin.hpp"
#include "components.hpp"
#define NUM_STEPS 8
#define NUM_SEQS 2

// [X] TODO - add reset input and button
// [ ] TODO - add clock input and button
// [ ] TODO - add step knob
// [ ] TODO - add cv 1v for both sets
// [ ] TODO - add cv 10v for both sets
// [ ] TODO - add cv inputs for both sets
// [ ] TODO - add main steps lights
// [ ] TODO - add probability knob
// [ ] TODO - add probability lights



struct Twinned2 : Module
{
	enum ParamId
	{
		P_TRIGBUTTON,
		P_RESETBUTTON,
		P_STEPSELECT,
		ENUMS(P_CV1, NUM_STEPS * 2),
		ENUMS(P_CV2, NUM_STEPS * 2),
		ENUMS(P_PROB, NUM_STEPS),
		PARAMS_LEN
	};

	enum InputId
	{
		I_CLOCK,
		I_RESET,
		ENUMS(I_CV, NUM_STEPS * 2),
		INPUTS_LEN
	};

	enum OutputId
	{
		O_CVWINNER,
		O_CVA,
		O_CVB,
		O_EOC,
		O_BGATE,
		O_AGATE,
		O_GATE,
		OUTPUTS_LEN
	};

	enum LightId
	{
		ENUMS(L_STEP, NUM_STEPS*2),
		LIGHTS_LEN
	};

	enum AB
	{
		A = 0,
		B = 8
	};


	dsp::ClockDivider _lowPriority;
	dsp::BooleanTrigger _resetButton;
	dsp::BooleanTrigger _clockButton;
	dsp::SchmittTrigger _resetTrigger;
	dsp::SchmittTrigger _clockTrigger;
	dsp::PulseGenerator _eocPulse;
	int _step = -1;
	int _lastStep = -1;
	int _num_steps=NUM_STEPS;
	bool _ready = true;



	Twinned2()
	{
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configButton(P_TRIGBUTTON, "Manual trigger");
		configButton(P_RESETBUTTON, "Reset");
		configInput(I_CLOCK, "Clock");
		configInput(I_RESET, "Reset");

		configParam(P_STEPSELECT, 1.f, NUM_STEPS, NUM_STEPS, "Steps");
		paramQuantities[P_STEPSELECT]->snapEnabled = true;

		for (int i = 0; i < NUM_STEPS; i++)
		{
			configParam(P_CV1 + i, 0.f, 1.f, 0.5f, string::f("Step A 1v %d", i + 1));
			configParam(P_CV1 + NUM_STEPS + i, 0.f, 1.f, 0.5f, string::f("Step B 1v %d", i + 1));

			configParam(P_CV2 + i, 0.f, 10.f, 0.f, string::f("Step A 10v %d", i + 1));
			configParam(P_CV2 + NUM_STEPS + i, 0.f, 10.f, 0.f, string::f("Step B 10v %d", i + 1));

			configParam(P_PROB + i, -0.f, 1.f, 0.5f, string::f("Prob %d", i + 1));
			configInput(I_CV + i, string::f("CV A %d", i + 1));
			configInput(I_CV + i + NUM_STEPS, string::f("CV B %d", i + 1));
		}
		configOutput(O_CVWINNER, "CV2 Winner Out");
		configOutput(O_CVA, "CV A Out");
		configOutput(O_CVB, "CV B Out");
		configOutput(O_EOC, "EOC Trigger");
		configOutput(O_GATE, "Gate");
		configOutput(O_AGATE, "Gate");
		configOutput(O_BGATE, "Gate");

		_lowPriority.setDivision(16);
		_ready = true;
		_step=0;
		_lastStep=0;
	}

	void onReset() override
	{
		_ready = true;
		_step = 0;
		_lastStep = 0;
		_num_steps = params[P_STEPSELECT].getValue();
	}

	float getCVOut(int step, int ab)
	{
		float cv;
		if(inputs[I_CV + step + ab].isConnected())
		{
			cv=inputs[I_CV + step + ab].getVoltage();
		}
		else
		{
			cv=params[P_CV1 + step + ab].getValue()+params[P_CV2 + step + ab].getValue();
		}
		return cv;
	}



	void process(const ProcessArgs &args) override
	{
		_num_steps=paramQuantities[P_STEPSELECT]->getValue();
		
		//clock trigger and button
		bool clock = _clockTrigger.process(inputs[I_CLOCK].getVoltage());
		bool clockbutton = _clockButton.process(params[P_TRIGBUTTON].getValue());
		if(_ready && (clock || clockbutton))
		{
			_step=(_step+1) % _num_steps;
			DEBUG("step: %d", _step);
			_ready=false;
		}
		_ready=!_clockTrigger.isHigh();

		//do step stuff
		if(_lastStep!=_step)
		{
			float prob=params[P_PROB+_step].getValue();
			int ab=(prob < random::uniform()) ? A : B;
			lights[L_STEP+_step+ab].setBrightness(1.f);
			float cvOut=getCVOut(_step, ab);
			outputs[O_CVWINNER].setVoltage(cvOut);
			outputs[O_CVA].setVoltage(inputs[I_CV+_step+A].getVoltage());
			outputs[O_CVB].setVoltage(inputs[I_CV+_step+B].getVoltage());

			lights[L_STEP+_lastStep].setBrightness(0.0f);
			lights[L_STEP+_lastStep+NUM_STEPS].setBrightness(0.0f);
			_lastStep=_step;

		}
	}

};



struct Twinned2Widget : ModuleWidget
{
	Twinned2Widget(Twinned2 *module)
	{
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Twinned2.svg")));

		// float width=121.92;
		float sy = 10;
		float sx = 10;
		float yOffset = 15;
		float xOffset = 10;
		float mx = 60;
		float x = xOffset;
		float y = yOffset;

		//clock trig input and button
		addInput(createInputCentered<CoffeeInputPortButton>(mm2px(Vec(x, y)), module, Twinned2::I_CLOCK));
		addParam(createParamCentered<CoffeeTinyButton>(mm2px(Vec(x + 3.5, y - 3.5)), module, Twinned2::P_TRIGBUTTON));

		// reset trig and button
		x += sx;
		addInput(createInputCentered<CoffeeInputPortButton>(mm2px(Vec(x, y)), module, Twinned2::I_RESET));
		addParam(createParamCentered<CoffeeTinyButton>(mm2px(Vec(x + 3.5, y - 3.5)), module, Twinned2::P_RESETBUTTON));

		// num steps stepped knob
		x += sx;
		addParam(createParamCentered<CoffeeKnob8mm>(mm2px(Vec(x, y)), module, Twinned2::P_STEPSELECT));

		// add cv inputs, cv1 and cv2 and prob knobs
		x=mx;
		float x1,x2;
		for(int i=0; i<NUM_STEPS; i++){
			y=yOffset+sy+(sy*i) ;
			//prob
			addParam(createParamCentered<CoffeeKnob6mm>(mm2px(Vec(x, y)), module, Twinned2::P_PROB + i));
			//lights
			addChild(createLightCentered<SmallLight<OrangeLight>>(mm2px(Vec(x-3.5, y+3.5)), module, Twinned2::L_STEP + i));
			addChild(createLightCentered<SmallLight<OrangeLight>>(mm2px(Vec(x+3.5, y+3.5)), module, Twinned2::L_STEP + NUM_STEPS + i));
			
			//inputs
			addInput(createInputCentered<CoffeeInputPort>(mm2px(Vec(x-sx, y)), module, Twinned2::I_CV + i));
			addInput(createInputCentered<CoffeeInputPort>(mm2px(Vec(x+sx, y)), module, Twinned2::I_CV + i + NUM_STEPS));
			//cv1
			addParam(createParamCentered<CoffeeKnob6mm>(mm2px(Vec(x-sx*2, y)), module, Twinned2::P_CV1 + i));
			addParam(createParamCentered<CoffeeKnob6mm>(mm2px(Vec(x+sx*2, y)), module, Twinned2::P_CV1 + i + NUM_STEPS));
			//cv2
			addParam(createParamCentered<CoffeeKnob4mm>(mm2px(Vec(x-sx*3, y)), module, Twinned2::P_CV2 + i));
			addParam(createParamCentered<CoffeeKnob4mm>(mm2px(Vec(x+sx*3, y)), module, Twinned2::P_CV2 + i + NUM_STEPS));

		}
		y=yOffset+sy+(sy*NUM_STEPS) ;
		//cv winner output
		addOutput(createOutputCentered<CoffeeOutputPort>(mm2px(Vec(x, y)), module, Twinned2::O_CVWINNER));
		//cv outputs
		addOutput(createOutputCentered<CoffeeOutputPort>(mm2px(Vec(x-sx, y)), module, Twinned2::O_CVA));
		addOutput(createOutputCentered<CoffeeOutputPort>(mm2px(Vec(x+sx, y)), module, Twinned2::O_CVB));

	}
};

Model *modelTwinned2 = createModel<Twinned2, Twinned2Widget>("Twinned2");