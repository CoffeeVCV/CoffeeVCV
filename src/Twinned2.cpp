#include "plugin.hpp"
#include "components.hpp"
#define NUM_STEPS 8
#define NUM_SEQS 2

// [X] TODO - add reset input and button
// [x] TODO - add clock input and button
// [x] TODO - add step knob
// [x] TODO - add cv 1v for both sets
// [x] TODO - add cv 10v for both sets
// [x] TODO - add cv inputs for both sets
// [x] TODO - add main steps lights
// [x] TODO - add probability knob
// [x] TODO - add probability lights 
// [ ] TODO - add gates knob and input for both sets
// [ ] TODO - add polyphonic input for both sets



struct Twinned2 : Module
{
	enum ParamId
	{
		P_TRIGBUTTON,
		P_RESETBUTTON,
		P_STEPSELECT,
		ENUMS(P_CV1, NUM_STEPS * NUM_SEQS),
		ENUMS(P_PROB, NUM_STEPS),
		ENUMS(P_GATE, NUM_STEPS * NUM_SEQS),
		PARAMS_LEN
	};

	enum InputId
	{
		I_CLOCK,
		I_RESET,
		ENUMS(I_CV, NUM_SEQS),
		ENUMS(I_GATE, NUM_SEQS),
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
	dsp::Timer _gateTimer[NUM_STEPS*2];
	float _tempo=0;
	float _lastframe=-1;	
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
			configParam(P_CV1 + NUM_STEPS + i, 0.f, 10.f, 0.5f, string::f("Step B %d", i + 1));

			configParam(P_PROB + i, -0.f, 1.f, 0.5f, string::f("Prob %d", i + 1));

			//gates
			configParam(P_GATE + i, 0.f, 1.f, 0.f, string::f("Gate A %d", i + 1));
			configParam(P_GATE + i + NUM_STEPS, 0.f, 1.f, 0.f, string::f("Gate B %d", i + 1));
		}

		configInput(I_CV + 0, "CV A");
		configInput(I_CV + 1, "CV B");
		configInput(I_GATE + 0, "GATES A");
		configInput(I_GATE + 1, "GATES B");		


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
		int i=(ab>0)?1:0;
		bool usePoly=true;
		usePoly &= inputs[I_CV + i].isConnected();
		usePoly &= inputs[I_CV + i].isPolyphonic();
		usePoly &= inputs[I_CV + i].getChannels() >= step;

		if(usePoly)
		{
			cv = inputs[I_CV + i].getPolyVoltage(step);
		}
		else
		{
			cv=params[P_CV1 + step + ab].getValue();
		}
		return cv;
	}



	void process(const ProcessArgs &args) override
	{
		//calculate tempo
		if (_lastframe > 0)
		{
			float delta = args.frame - _lastframe;
			float rate = delta / args.sampleRate;
			_tempo = 60 / rate;
			//DEBUG("Tempo: %f",_tempo);				
		}
		_lastframe = args.frame;

		_num_steps=paramQuantities[P_STEPSELECT]->getValue();
		
		//clock trigger and button
		bool clock = _clockTrigger.process(inputs[I_CLOCK].getVoltage());
		bool clockbutton = _clockButton.process(params[P_TRIGBUTTON].getValue());
		if(_ready && (clock || clockbutton))
		{
			_step=(_step+1) % _num_steps;
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

			outputs[O_CVA].setVoltage(getCVOut(_step, A));
			outputs[O_CVB].setVoltage(getCVOut(_step, B));

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

		// end of cycle output
		x += sx;
		addOutput(createOutputCentered<CoffeeOutputPort>(mm2px(Vec(x, y)), module, Twinned2::O_EOC));

		// add cv inputs, cv1 and cv2 and prob knobs
		x=mx;

		//poly inputs CV
		y=yOffset+sy;
		addInput(createInputCentered<CoffeeInputPort>(mm2px(Vec(x-sx, y)), module, Twinned2::I_CV + 0));
		addInput(createInputCentered<CoffeeInputPort>(mm2px(Vec(x+sx, y)), module, Twinned2::I_CV + 1));

		//poly inputs GATES
		addInput(createInputCentered<CoffeeInputPort>(mm2px(Vec(x-sx*2, y)), module, Twinned2::I_GATE + 0));
		addInput(createInputCentered<CoffeeInputPort>(mm2px(Vec(x+sx*2, y)), module, Twinned2::I_GATE + 1));

		for(int i=0; i<NUM_STEPS; i++){
			y=yOffset+sy+sy+(sy*i) ;
			//prob
			addParam(createParamCentered<CoffeeKnob6mm>(mm2px(Vec(x, y)), module, Twinned2::P_PROB + i));
			//lights
			addChild(createLightCentered<SmallLight<OrangeLight>>(mm2px(Vec(x-3.5, y+3.5)), module, Twinned2::L_STEP + i));
			addChild(createLightCentered<SmallLight<OrangeLight>>(mm2px(Vec(x+3.5, y+3.5)), module, Twinned2::L_STEP + NUM_STEPS + i));
			//cv1
			addParam(createParamCentered<CoffeeKnob6mm>(mm2px(Vec(x-sx, y)), module, Twinned2::P_CV1 + i));
			addParam(createParamCentered<CoffeeKnob6mm>(mm2px(Vec(x+sx, y)), module, Twinned2::P_CV1 + i + NUM_STEPS));
			//gate knobs
			addParam(createParamCentered<CoffeeKnob6mm>(mm2px(Vec(x-sx*2, y)), module, Twinned2::P_GATE + i));
			addParam(createParamCentered<CoffeeKnob6mm>(mm2px(Vec(x+sx*2, y)), module, Twinned2::P_GATE + i + NUM_STEPS));
		}

		y=yOffset+sy+sy+(sy*NUM_STEPS) ;
		//cv winner output
		addOutput(createOutputCentered<CoffeeOutputPort>(mm2px(Vec(x, y)), module, Twinned2::O_CVWINNER));
		//cv outputs
		addOutput(createOutputCentered<CoffeeOutputPort>(mm2px(Vec(x-sx, y)), module, Twinned2::O_CVA));
		addOutput(createOutputCentered<CoffeeOutputPort>(mm2px(Vec(x+sx, y)), module, Twinned2::O_CVB));

		//gate outputs
		addOutput(createOutputCentered<CoffeeOutputPort>(mm2px(Vec(x-sx*2, y)), module, Twinned2::O_AGATE));
		addOutput(createOutputCentered<CoffeeOutputPort>(mm2px(Vec(x+sx*2, y)), module, Twinned2::O_BGATE));



	}





};

Model *modelTwinned2 = createModel<Twinned2, Twinned2Widget>("Twinned2");