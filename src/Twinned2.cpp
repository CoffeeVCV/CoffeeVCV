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
// [x] TODO - add gates knob, and make gates a poly output
// [x] TODO - add polyphonic input for both sets
// [x] TODO - add menu to copy from A to B, or b to A
// [X] TODO - update to/from json functions
// [x] TODO - add a 2nd cv param for both sets
// [X] TODo - fix menus
// [ ] TODO - add other mode toggles : input * rand(v1) * v2
// [ ] TODO - defend Steps from randomised values
// [X] TODO - add steps input and range selection

struct Twinned2 : Module
{
	enum ParamId
	{
		P_TRIGBUTTON,
		P_RESETBUTTON,
		P_STEPSELECT,
		P_ABTHRESHOLD,
		ENUMS(P_VOCT1, NUM_STEPS *NUM_SEQS),
		ENUMS(P_VOCT2, NUM_STEPS *NUM_SEQS),
		ENUMS(P_PROB, NUM_STEPS),
		ENUMS(P_GATE, NUM_STEPS *NUM_SEQS),
		P_RANDOMIZESCALE,
		ENUMS(P_RANDBUTTON, NUM_SEQS * 2),
		PARAMS_LEN
	};

	enum InputId
	{
		I_CLOCK,
		I_RESET,
		I_ABSELECT,
		I_STEPSELECT,
		ENUMS(I_VOCT, NUM_SEQS),
		ENUMS(I_GATE, NUM_SEQS),
		ENUMS(I_RAND, NUM_SEQS * 2),
		INPUTS_LEN
	};

	enum OutputId
	{
		O_CVWINNER,
		O_VOCTA,
		O_VOCTB,
		O_EOC,
		O_BGATE,
		O_AGATE,
		O_GATE,
		OUTPUTS_LEN
	};

	enum LightId
	{
		ENUMS(L_STEP, NUM_STEPS * 2),
		L_ERRSTEPS,
		LIGHTS_LEN
	};

	enum AB
	{
		A = 0,
		B = 8
	};

	enum actions
	{
		VOCT1AtoB,
		VOCT1BtoA,
		VOCT2AtoB,
		VOCT2BtoA,
		GA2B,
		GB2A
	};

	enum RANDCONTROLS
	{
		VOCTA,
		VOCTB,
		GATEA,
		GATEB,
		V1ONLY,
		V2ONLY,
		V1ANDV2
	};

	enum RANDMASK
	{
		RANDNOTE = 1,
		RANDOCT = 2,
		RANDGATES = 4,
		RANDPROB = 8,
		RANDSTEP = 16
	};

	dsp::ClockDivider _lowPriority;
	dsp::BooleanTrigger _randButtons[NUM_SEQS * 2];
	dsp::SchmittTrigger _randTrigger[NUM_SEQS * 2];
	dsp::BooleanTrigger _resetButton;
	dsp::BooleanTrigger _clockButton;
	dsp::SchmittTrigger _resetTrigger;
	dsp::SchmittTrigger _clockTrigger;
	dsp::PulseGenerator _eocPulse;
	dsp::Timer _gateTimer[NUM_STEPS * 2];
	dsp::Timer _gate;
	float _tempo = 0;
	float _lastframe = -1;
	int _step = -1;
	int _lastStep = -1;
	int _lastAB = 0;
	int _num_steps = -1;
	int _last_num_steps = -1;
	bool _ready = true;
	int _ab = A;
	bool _menu_polyGates = false;
	int _menu_copyAction = -1;
	bool _menu_updateControlsFromPoly = true;
	int _menu_randomizeMode = V1ONLY;
	float _menu_stepInputScales[3] = {1.f, 8.f, 10.f};
	int _menu_stepInputScale = 2;
	int _menu_randMask = 13;

	Twinned2()
	{
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configButton(P_TRIGBUTTON, "Manual trigger");
		configButton(P_RESETBUTTON, "Reset");
		configInput(I_CLOCK, "Clock");
		configInput(I_RESET, "Reset");
		configParam(P_RANDOMIZESCALE, 0.f, 1.f, 0.05f, "Randomize Amount", "%", 0.f, 100.f);
		configParam(P_STEPSELECT, 1.f, NUM_STEPS, NUM_STEPS, "Steps");
		paramQuantities[P_STEPSELECT]->snapEnabled = true;
		configParam(P_ABTHRESHOLD, 0.f, 10.f, 5.f, "AB Threshold");
		for (int i = 0; i < NUM_STEPS; i++)
		{
			configParam(P_VOCT1 + i, 0.f, 1.f, 0.0f, string::f("Step A Note %d", i + 1));
			configParam(P_VOCT1 + NUM_STEPS + i, 0.f, 1.f, 0.f, string::f("Step B Note %d", i + 1));

			configParam(P_VOCT2 + i, -10.f, 10.f, 0.0f, string::f("Step A Oct %d", i + 1));
			paramQuantities[P_VOCT2 + i]->snapEnabled = true;
			configParam(P_VOCT2 + NUM_STEPS + i, -10.f, 10.f, 0.f, string::f("Step B Oct %d", i + 1));
			paramQuantities[P_VOCT2 + NUM_STEPS + i]->snapEnabled = true;

			configParam(P_PROB + i, -0.f, 1.f, 0.5f, string::f("Prob %d", i + 1));

			// gates
			configParam(P_GATE + i, 0.f, 1.f, 0.5f, string::f("Gate A %d", i + 1), "%", 0.f, 100.0f);
			configParam(P_GATE + i + NUM_STEPS, 0.f, 1.f, 0.5f, string::f("Gate B %d", i + 1), "%", 0.f, 100.0f);
		}

		configButton(P_RANDBUTTON + VOCTA, "Randomize A V/OCT");
		configButton(P_RANDBUTTON + VOCTB, "Randomize B V/OCT");
		configButton(P_RANDBUTTON + GATEA, "Randomize A Gates");
		configButton(P_RANDBUTTON + GATEB, "Randomize B Gates");

		configInput(I_RAND + VOCTA, "Randomize A V/OCT");
		configInput(I_RAND + VOCTB, "Randomize B V/OCT");
		configInput(I_RAND + GATEA, "Randomize A Gates");
		configInput(I_RAND + GATEB, "Randomize B Gates");

		configInput(I_STEPSELECT, "Steps");
		configInput(I_ABSELECT, "AB Select");
		configInput(I_VOCT + 0, "V/OCT A");
		configInput(I_VOCT + 1, "V/OCT B");
		configInput(I_GATE + 0, "GATES A");
		configInput(I_GATE + 1, "GATES B");

		configOutput(O_CVWINNER, "V/OCT Selected Out");
		configOutput(O_VOCTA, "V/OCT A Out");
		configOutput(O_VOCTB, "V/OCT B Out");
		configOutput(O_EOC, "EOC Trigger");
		configOutput(O_GATE, "Selected Gate");
		configOutput(O_AGATE, "A Gate");
		configOutput(O_BGATE, "B Gate");

		_lowPriority.setDivision(32);
		_ready = true;
		_step = 0;
		_lastStep = 0;
	}

	void onReset() override
	{
		_ready = true;
		_step = 0;
		_lastStep = 0;
		_num_steps = params[P_STEPSELECT].getValue();
	}

	void onRandomize() override {
		DEBUG("onRandomize");
		for(int i=0; i<NUM_STEPS; i++){
			if(_menu_randMask & RANDNOTE) {
				params[P_VOCT1 + i].setValue(random::uniform());
				params[P_VOCT1 + NUM_STEPS + i].setValue(random::uniform());
			}
			if(_menu_randMask & RANDOCT) {
				params[P_VOCT2 + i].setValue(random::uniform() * 20.f - 10.f);
				params[P_VOCT2 + NUM_STEPS + i].setValue(random::uniform() * 20.f - 10.f);
			}
			if(_menu_randMask & RANDPROB) {
				params[P_PROB + i].setValue(random::uniform());
			}
			if(_menu_randMask & RANDGATES) {
				params[P_GATE + i].setValue(random::uniform());
				params[P_GATE + i + NUM_STEPS].setValue(random::uniform());
			}
		}
		if(_menu_randMask & RANDSTEP) {
			params[P_STEPSELECT].setValue((random::uniform() * (NUM_STEPS-1))+1);
		}	
	}

	float getCVOut(int step, int ab)
	{
		float cv;
		int i = (ab > 0) ? 1 : 0;
		bool usePoly = true;
		usePoly &= inputs[I_VOCT + i].isConnected();
		usePoly &= inputs[I_VOCT + i].isPolyphonic();
		usePoly &= inputs[I_VOCT + i].getChannels() >= step;

		if (usePoly)
		{
			cv = inputs[I_VOCT + i].getPolyVoltage(step);
		}
		else
		{
			cv = params[P_VOCT1 + step + ab].getValue() + params[P_VOCT2 + step + ab].getValue();
		}
		return cv;
	}

	float getScaledRandom(ParamQuantity *q, float scale, float oldValue)
	{
		float min = q->getMinValue();
		float max = q->getMaxValue();
		float range = max - min;
		float random = random::uniform();
		float newValue = range * scale * random;
		newValue = clamp((random::uniform() >= 0.5) ? oldValue + newValue : oldValue - newValue, min, max);
		return newValue;
	}

	json_t *dataToJson() override
	{
		//save menu items to jason
		json_t *rootJ = json_object();

		json_t *polyGates = json_boolean(_menu_polyGates);
		json_object_set_new(rootJ, "polyGates", polyGates);

		json_t *stepInputScale = json_real(_menu_stepInputScale);
		json_object_set_new(rootJ, "stepInputScale", stepInputScale);

		json_t *randomizeMode = json_integer(_menu_randomizeMode);
		json_object_set_new(rootJ, "randomizeMode", randomizeMode);

		json_t *updateControlsFromPoly = json_integer(_menu_updateControlsFromPoly);
		json_object_set_new(rootJ, "updateControlsFromPoly", updateControlsFromPoly);

		json_t *randMask = json_integer(_menu_randMask);
		json_object_set_new(rootJ, "randMask", randMask);

		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override
	{
		json_t *polyGates = json_object_get(rootJ, "polyGates");
		if (polyGates)
		{
			_menu_polyGates = json_boolean_value(polyGates);
		}

		json_t *stepInputScale = json_object_get(rootJ, "stepInputScale");
		if (stepInputScale)
		{
			_menu_stepInputScale = json_real_value(stepInputScale);
		}

		json_t *randomizeMode = json_object_get(rootJ, "randomizeMode");
		if (randomizeMode)
		{
			_menu_randomizeMode = json_integer_value(randomizeMode);
		}

		json_t *updateControlsFromPoly = json_object_get(rootJ, "updateControlsFromPoly");
		if (updateControlsFromPoly)
		{
			_menu_updateControlsFromPoly = json_integer_value(updateControlsFromPoly);
		}	
		
		json_t *randMask = json_object_get(rootJ, "randMask");
		if (randMask)
		{
			_menu_randMask = json_integer_value(randMask);
		}	
	}

	void process(const ProcessArgs &args) override
	{
		if (_lowPriority.process())
		{
			//check if input is being used for step num
			if (inputs[I_STEPSELECT].isConnected())
			{
				_num_steps = rescale(inputs[I_STEPSELECT].getVoltage(),0.f,_menu_stepInputScales[_menu_stepInputScale],0,NUM_STEPS);
				if(_num_steps <1 || _num_steps > NUM_STEPS)
				{
					DEBUG("num step OOB %d", _num_steps);
					lights[L_ERRSTEPS].setBrightness(1.0f);
					_num_steps=clamp(_num_steps,1,NUM_STEPS);
				} else {
					lights[L_ERRSTEPS].setBrightness(0.0f);
				}
			}
			else
			{
				_num_steps = params[P_STEPSELECT].getValue();
			}

			if (_num_steps != _last_num_steps)
			{
				_step = 0;
				_lastStep = 0;
				_last_num_steps=_num_steps;
				for (int i = 0; i < NUM_STEPS * 2; i++)
				{
					lights[L_STEP + i].setBrightness(0.f);
				}
			}

			int channels = (_menu_polyGates) ? _num_steps : 1;
			outputs[O_AGATE].setChannels(channels);
			outputs[O_BGATE].setChannels(channels);
			outputs[O_GATE].setChannels(channels);

			if (_menu_copyAction >= 0)
			{
				for (int i = 0; i < NUM_STEPS; i++)
				{
					if (_menu_copyAction == VOCT1AtoB)
					{
						params[P_VOCT1 + i + B].setValue(params[P_VOCT1 + i + A].getValue());
					}
					else if (_menu_copyAction == VOCT1BtoA)
					{
						params[P_VOCT1 + i + A].setValue(params[P_VOCT1 + i + B].getValue());
					}					
					else if (_menu_copyAction == VOCT2AtoB)
					{
						params[P_VOCT2 + i + B].setValue(params[P_VOCT2 + i + A].getValue());
					}
					else if (_menu_copyAction == VOCT2BtoA)
					{
						params[P_VOCT2 + i + A].setValue(params[P_VOCT2 + i + B].getValue());
					}
					else if (_menu_copyAction == GA2B)
					{
						params[P_GATE + i + B].setValue(params[P_GATE + i + A].getValue());
					}
					else if (_menu_copyAction == GB2A)
					{
						params[P_GATE + i + A].setValue(params[P_GATE + i + B].getValue());
					}
					_menu_copyAction = -1;
				}
			}

			// update gates if poly in is used
			if (_menu_updateControlsFromPoly)
			{
				for (int seq = 0; seq < NUM_SEQS; seq++)
				{
					// update voct knobs
					if (inputs[I_VOCT + seq].isConnected() && inputs[I_VOCT + seq].isPolyphonic())
					{
						for (int i = 0; i < inputs[I_VOCT + seq].getChannels(); i++)
						{
							int v=inputs[I_VOCT + seq].getPolyVoltage(i);
							params[P_VOCT2 + i + (seq * NUM_STEPS)].setValue(v);
							params[P_VOCT1 + i + (seq * NUM_STEPS)].setValue(inputs[I_VOCT + seq].getPolyVoltage(i)-v);
						}
					}
					// update gate knobs
					if (inputs[I_GATE + seq].isConnected() && inputs[I_GATE + seq].isPolyphonic())
					{
						for (int i = 0; i < inputs[I_GATE + seq].getChannels(); i++)
						{
							params[P_GATE + i + (seq * NUM_STEPS)].setValue(inputs[I_GATE + seq].getPolyVoltage(i));
						}
					}
				}
			}
		} // end low priority

		// check if any of the randomisation is needed.
		float scale = params[P_RANDOMIZESCALE].getValue();
		for (int i = 0; i < NUM_SEQS * 2; i++)  // 4 possible randomise tiggers
		{
			bool buttonPressed = _randButtons[i].process(params[P_RANDBUTTON + i].getValue());
			bool inputPressed = _randTrigger[i].process(inputs[I_RAND + i].getVoltage());
			if (buttonPressed || inputPressed)
			{
				for (int j = 0; j < NUM_STEPS; j++)
				{
					int pid1;
					int pid2=-1;
					if (i == VOCTA)
						pid1 = P_VOCT1 + j;  //note 
						pid2 = P_VOCT2 + j;  //octave
					if (i == VOCTB)
						pid1 = P_VOCT1 + j + NUM_STEPS;  //note
						pid2 = P_VOCT2 + j + NUM_STEPS;  //octave
					if (i == GATEA)
						pid1 = P_GATE + j;
					if (i == GATEB)
						pid1 = P_GATE + j + NUM_STEPS;

					if(i==GATEA || i==GATEB) {  //gates 
						float oldValue = params[pid1].getValue();
						float newValue = getScaledRandom(paramQuantities[pid1], scale, oldValue);
						params[pid1].setValue(newValue);
					} else {  // voct is done differently
						if(_menu_randomizeMode == V1ONLY ) {
							float oldValue = params[pid1].getValue();
							float newValue = getScaledRandom(paramQuantities[pid1], scale, oldValue);
							params[pid1].setValue(newValue);
						} else if (_menu_randomizeMode == V1ANDV2) {
							float oldValue1 = params[pid1].getValue();
							float oldValue2 = params[pid2].getValue();
							float newValue1 = getScaledRandom(paramQuantities[pid1], scale, oldValue1 + oldValue2);
							int v2 = (int)newValue1;
							params[pid1].setValue(newValue1-v2);
							params[pid2].setValue(v2);
						}
					}
				}
			}
		}

		// clock trigger and button
		bool clock = _clockTrigger.process(inputs[I_CLOCK].getVoltage());
		bool clockbutton = _clockButton.process(params[P_TRIGBUTTON].getValue());
		if (_ready && (clock || clockbutton))
		{
			_step = (_step + 1) % _num_steps;
			_ready = false;

			// calculate tempo, so that we can calculate the gate time
			if (_lastframe > 0)
			{
				float delta = args.frame - _lastframe;
				float rate = delta / args.sampleRate;
				_tempo = 60 / rate;
			}
			_lastframe = args.frame;
		}
		_ready = !_clockTrigger.isHigh();

		// check eoc
		bool eoc = _eocPulse.process(args.sampleTime);
		outputs[O_EOC].setVoltage((eoc) ? 10.f : 0.f);

		// check gate timers and close any expired gates
		for (int j = 0; j < 2; j++)
		{
			// first As then Bs
			for (int i = 0; i < _num_steps; i++)
			{
				int g = i + (j * NUM_STEPS);

				// // gates's elapsed time
				float timeElapsed = _gateTimer[g].process(args.sampleTime);
				// // gate param is a percentage of a beat
				float targetTime = params[P_GATE + g].getValue() / (_tempo / 60);
				bool timerExpired = timeElapsed >= targetTime;
				// //if(!timerExpired)
				// 	//DEBUG("gate %d: %f / %f", g, timeElapsed, targetTime);

				if (timerExpired)
				{
					int channel = (_menu_polyGates) ? i : 0;
					int ABGate = (j == A) ? O_AGATE : O_BGATE;

					if (_menu_polyGates || i == _step)
					{
						outputs[ABGate].setVoltage(0, channel);
					}

					// if this is the last winning sequence's gate expiring
					if (j * NUM_STEPS == _lastAB && i == _lastStep)
					{
						outputs[O_GATE].setVoltage(0, channel);
					}
				}
			}
		}

		// do step stuff
		if (_lastStep != _step)
		{
			int ab;
			if (inputs[I_ABSELECT].isConnected())
			{
				ab = (inputs[I_ABSELECT].getVoltage() < params[P_ABTHRESHOLD].getValue()) ? A : B;
			}
			else
			{
				float prob = params[P_PROB + _step].getValue();
				ab = (prob < random::uniform()) ? A : B;
			}

			float cvOut = getCVOut(_step, ab);
			outputs[O_CVWINNER].setVoltage(cvOut);
			outputs[O_VOCTA].setVoltage(getCVOut(_step, A));
			outputs[O_VOCTB].setVoltage(getCVOut(_step, B));

			// start gate timers
			_gateTimer[_step + A].reset();
			_gateTimer[_step + B].reset();

			// open gates

			int channel = (_menu_polyGates) ? _step : 0;
			// DEBUG("channel %d", channel);
			outputs[O_GATE].setVoltage(10.f, channel);
			outputs[O_AGATE].setVoltage(10.f, channel);
			outputs[O_BGATE].setVoltage(10.f, channel);

			// lights on
			lights[L_STEP + _step + ab].setBrightness(1.f);

			// lights off
			lights[L_STEP + _lastStep].setBrightness(0.0f);
			lights[L_STEP + _lastStep + NUM_STEPS].setBrightness(0.0f);

			// end of cycle
			if (_step == _num_steps - 1)
			{
				outputs[O_EOC].setVoltage(10.f);
				_eocPulse.trigger(1e-3);
			}

			// next step
			_lastStep = _step;
			_lastAB = ab;
			//_step++;
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
		float xOffset = sx / 4;
		float mx = 81.12 / 2;

		float y = yOffset + sy * 2;
		float x = mx - (sx * 3) - 5;
		// clock trig input and button
		addInput(createInputCentered<CoffeeInputPortButton>(mm2px(Vec(x, y)), module, Twinned2::I_CLOCK));
		addParam(createParamCentered<CoffeeTinyButton>(mm2px(Vec(x + 3.5, y - 3.5)), module, Twinned2::P_TRIGBUTTON));

		// reset trig and button
		// x += sx;
		y+=sy;
		addInput(createInputCentered<CoffeeInputPortButton>(mm2px(Vec(x, y)), module, Twinned2::I_RESET));
		addParam(createParamCentered<CoffeeTinyButton>(mm2px(Vec(x + 3.5, y - 3.5)), module, Twinned2::P_RESETBUTTON));

		//steps switch, input and indicator
		y+=sy;
		addInput(createInputCentered<CoffeeInputPortIndicator>(mm2px(Vec(x, y)), module, Twinned2::I_STEPSELECT));
		addChild(createLightCentered<SmallLight<RedLight> >(mm2px(Vec(x + 3.5, y + 3.5)), module, Twinned2::L_ERRSTEPS));

		y+=sy;
		// num steps stepped knob
		addParam(createParamCentered<CoffeeKnob8mm>(mm2px(Vec(x, y)), module, Twinned2::P_STEPSELECT));

		y+=sy+sy;
		// randomize scale knob
		addParam(createParamCentered<CoffeeKnob8mm>(mm2px(Vec(x, y)), module, Twinned2::P_RANDOMIZESCALE));

		y = yOffset - (sy / 4);
		// threshold input and knob
		x = mx;
		addInput(createInputCentered<CoffeeInputPort>(mm2px(Vec(x, y)), module, Twinned2::I_ABSELECT));
		addParam(createParamCentered<CoffeeKnob8mm>(mm2px(Vec(x, y + sy)), module, Twinned2::P_ABTHRESHOLD));

		// poly inputs CV
		y += sy;
		addInput(createInputCentered<CoffeeInputPort>(mm2px(Vec(x - sx - xOffset, y)), module, Twinned2::I_VOCT + 0));
		addInput(createInputCentered<CoffeeInputPort>(mm2px(Vec(x + sx + xOffset, y)), module, Twinned2::I_VOCT + 1));

		// poly inputs GATES
		addInput(createInputCentered<CoffeeInputPort>(mm2px(Vec(x - xOffset - sx * 2, y)), module, Twinned2::I_GATE + 0));
		addInput(createInputCentered<CoffeeInputPort>(mm2px(Vec(x + xOffset + sx * 2, y)), module, Twinned2::I_GATE + 1));

		// rand inputs and manula buttons
		x = mx;
		y = yOffset - (sy / 4);
		addInput(createInputCentered<CoffeeInputPortButton>(mm2px(Vec(x - sx - xOffset, y)), module, Twinned2::I_RAND + Twinned2::VOCTA));
		addParam(createParamCentered<CoffeeTinyButton>(mm2px(Vec(x - sx - xOffset + 3.5, y - 3.5)), module, Twinned2::P_RANDBUTTON + Twinned2::VOCTA));

		addInput(createInputCentered<CoffeeInputPortButton>(mm2px(Vec(x + sx + xOffset, y)), module, Twinned2::I_RAND + Twinned2::VOCTB));
		addParam(createParamCentered<CoffeeTinyButton>(mm2px(Vec(x + sx + xOffset + 3.5, y - 3.5)), module, Twinned2::P_RANDBUTTON + Twinned2::VOCTB));

		addInput(createInputCentered<CoffeeInputPortButton>(mm2px(Vec(x - sx - sx - xOffset, y)), module, Twinned2::I_RAND + Twinned2::GATEA));
		addParam(createParamCentered<CoffeeTinyButton>(mm2px(Vec(x - sx - sx - xOffset + 3.5, y - 3.5)), module, Twinned2::P_RANDBUTTON + Twinned2::GATEA));

		addInput(createInputCentered<CoffeeInputPortButton>(mm2px(Vec(x + sx + sx + xOffset, y)), module, Twinned2::I_RAND + Twinned2::GATEB));
		addParam(createParamCentered<CoffeeTinyButton>(mm2px(Vec(x + sx + sx + +xOffset + 3.5, y - 3.5)), module, Twinned2::P_RANDBUTTON + Twinned2::GATEB));

		x = mx;
		for (int i = 0; i < NUM_STEPS; i++)
		{
			y = yOffset + sy + sy + (sy * i);
			// prob
			addParam(createParamCentered<CoffeeKnob6mm>(mm2px(Vec(x, y)), module, Twinned2::P_PROB + i));
			// lights
			addChild(createLightCentered<SmallLight<OrangeLight> >(mm2px(Vec(x - 3.5, y + 3.5)), module, Twinned2::L_STEP + i));
			addChild(createLightCentered<SmallLight<OrangeLight> >(mm2px(Vec(x + 3.5, y + 3.5)), module, Twinned2::L_STEP + NUM_STEPS + i));

			// voct1
			addParam(createParamCentered<CoffeeKnob6mm>(mm2px(Vec(x - sx - xOffset + 2, y)), module, Twinned2::P_VOCT1 + i));
			addParam(createParamCentered<CoffeeKnob6mm>(mm2px(Vec(x + sx + xOffset - 2, y)), module, Twinned2::P_VOCT1 + i + NUM_STEPS));
			// voct2
			addParam(createParamCentered<CoffeeKnob4mm>(mm2px(Vec(x - sx - 3 - 2.5, y + 2.5)), module, Twinned2::P_VOCT2 + i));
			addParam(createParamCentered<CoffeeKnob4mm>(mm2px(Vec(x + sx + 3 + 2.5, y + 2.5)), module, Twinned2::P_VOCT2 + i + NUM_STEPS));

			// gate knobs
			addParam(createParamCentered<CoffeeKnob6mm>(mm2px(Vec(x - xOffset - sx * 2, y)), module, Twinned2::P_GATE + i));
			addParam(createParamCentered<CoffeeKnob6mm>(mm2px(Vec(x + xOffset + sx * 2, y)), module, Twinned2::P_GATE + i + NUM_STEPS));
		}

		y = yOffset + sy + sy + (sy * NUM_STEPS);
		// cv winner output
		addOutput(createOutputCentered<CoffeeOutputPort>(mm2px(Vec(x, y)), module, Twinned2::O_CVWINNER));
		// cv outputs

		addOutput(createOutputCentered<CoffeeOutputPort>(mm2px(Vec(x - sx - xOffset, y)), module, Twinned2::O_VOCTA));
		addOutput(createOutputCentered<CoffeeOutputPort>(mm2px(Vec(x + sx + xOffset, y)), module, Twinned2::O_VOCTB));

		// gate outputs
		y = yOffset + (sy * 6);
		x += 5 + sx * 3;
		addOutput(createOutputCentered<CoffeeOutputPort>(mm2px(Vec(x, y)), module, Twinned2::O_AGATE));
		y += sy;
		addOutput(createOutputCentered<CoffeeOutputPort>(mm2px(Vec(x, y)), module, Twinned2::O_BGATE));
		y += sy;
		addOutput(createOutputCentered<CoffeeOutputPort>(mm2px(Vec(x, y)), module, Twinned2::O_GATE));
		// end of cycle output
		y += sy;
		addOutput(createOutputCentered<CoffeeOutputPort>(mm2px(Vec(x, y)), module, Twinned2::O_EOC));
	}
	void appendContextMenu(Menu *menu) override
	{
		Twinned2 *module = dynamic_cast<Twinned2 *>(this->module);
		assert(module);
		menu->addChild(new MenuSeparator());
		menu->addChild(createSubmenuItem("Polyphony", "", [=](Menu *menu) {
			Menu* PolySelectMenu = new Menu();
			PolySelectMenu->addChild(createMenuItem("Polyphonic Gate Out", CHECKMARK(module->_menu_polyGates == true), [module]() { module->_menu_polyGates = !module->_menu_polyGates; }));
			PolySelectMenu->addChild(createMenuItem("Update knobs from polyphony inputs", CHECKMARK(module->_menu_updateControlsFromPoly == true), [module]() { module->_menu_updateControlsFromPoly = !module->_menu_updateControlsFromPoly; }));
			menu->addChild(PolySelectMenu); }));
		
		menu->addChild(createSubmenuItem("Copy values", "", [=](Menu *menu) {
			Menu* CopyMenu = new Menu();
			CopyMenu->addChild(createMenuItem("Copy A V1 -> B", CHECKMARK(module->_menu_copyAction == Twinned2::VOCT1AtoB), [module]() { module->_menu_copyAction = Twinned2::VOCT1AtoB; }));
			CopyMenu->addChild(createMenuItem("Copy A V2 -> B", CHECKMARK(module->_menu_copyAction == Twinned2::VOCT2AtoB), [module]() { module->_menu_copyAction = Twinned2::VOCT2AtoB; }));
			CopyMenu->addChild(createMenuItem("Copy B V1 -> A", CHECKMARK(module->_menu_copyAction == Twinned2::VOCT1BtoA), [module]() { module->_menu_copyAction = Twinned2::VOCT1BtoA; }));
			CopyMenu->addChild(createMenuItem("Copy B V2 -> A", CHECKMARK(module->_menu_copyAction == Twinned2::VOCT2BtoA), [module]() { module->_menu_copyAction = Twinned2::VOCT2BtoA; }));
			CopyMenu->addChild(createMenuItem("Copy A Gates -> B", CHECKMARK(module->_menu_copyAction == Twinned2::GA2B), [module]() { module->_menu_copyAction = Twinned2::GA2B; }));
			CopyMenu->addChild(createMenuItem("Copy B Gates -> A", CHECKMARK(module->_menu_copyAction == Twinned2::GB2A), [module]() { module->_menu_copyAction = Twinned2::GB2A; }));
			menu->addChild(CopyMenu); }));
		
		menu->addChild(createSubmenuItem("Randomize Input Trigger", "", [=](Menu *menu) {
			Menu* RandomizeMenu = new Menu();
			RandomizeMenu->addChild(createMenuItem("Randomize Notes only", CHECKMARK(module->_menu_randomizeMode == Twinned2::V1ONLY), [module]() { module->_menu_randomizeMode = Twinned2::V1ONLY; }));
			RandomizeMenu->addChild(createMenuItem("Randomize Notes and Octave", CHECKMARK(module->_menu_randomizeMode == Twinned2::V1ANDV2), [module]() { module->_menu_randomizeMode = Twinned2::V1ANDV2; }));
			menu->addChild(RandomizeMenu); }));

		menu->addChild(createSubmenuItem("Module Randomization", "", [=](Menu *menu) {
			Menu* RandomizationMenu = new Menu();
			RandomizationMenu->addChild(createMenuItem("Include V1 (Notes)", CHECKMARK(module->_menu_randMask & Twinned2::RANDNOTE), [module]() { module->_menu_randMask^=Twinned2::RANDNOTE; }));
			RandomizationMenu->addChild(createMenuItem("Include V2 (Octavess)", CHECKMARK(module->_menu_randMask & Twinned2::RANDOCT), [module]() { module->_menu_randMask^=Twinned2::RANDOCT; }));
			RandomizationMenu->addChild(createMenuItem("Include Gates", CHECKMARK(module->_menu_randMask & Twinned2::RANDGATES), [module]() { module->_menu_randMask^=Twinned2::RANDGATES; }));
			RandomizationMenu->addChild(createMenuItem("Include Probability", CHECKMARK(module->_menu_randMask & Twinned2::RANDPROB), [module]() { module->_menu_randMask^=Twinned2::RANDPROB; }));
			RandomizationMenu->addChild(createMenuItem("Include Steps", CHECKMARK(module->_menu_randMask & Twinned2::RANDSTEP), [module]() { module->_menu_randMask^=Twinned2::RANDSTEP; }));
			menu->addChild(RandomizationMenu); }));

		menu->addChild(createSubmenuItem("Step Input Scale", "", [=](Menu *menu) {
			Menu* ScaleMenu = new Menu();
			ScaleMenu->addChild(createMenuItem("0.0 to 1.0", CHECKMARK(module->_menu_stepInputScale == 0), [module]() { module->_menu_stepInputScale = 0; }));
			ScaleMenu->addChild(createMenuItem("0.0 to 8.0", CHECKMARK(module->_menu_stepInputScale == 1), [module]() { module->_menu_stepInputScale = 1; }));
			ScaleMenu->addChild(createMenuItem("0.0 to 10.0", CHECKMARK(module->_menu_stepInputScale == 2), [module]() { module->_menu_stepInputScale = 2; }));
			menu->addChild(ScaleMenu); }));	};
};

Model *modelTwinned2 = createModel<Twinned2, Twinned2Widget>("Twinned2");