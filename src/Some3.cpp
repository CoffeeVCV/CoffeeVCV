#include "plugin.hpp"
#include "components.hpp"
#define NUM_CHANNELS 16

struct Some3 : Module
{
	enum ParamId
	{
		P_TRIGBUTTON,
		P_SHIFTUPBUTTON,
		P_SHIFTDOWNBUTTON,
		P_SELECTIONSTART,
		P_SELECTIONEND,
		P_PROB,
		PARAMS_LEN
	};
	enum InputId
	{
		I_TRIG,
		I_PROB,
		I_SELECTIONSTART,
		I_SELECTIONEND,
		I_SEED,
		I_CV,
		I_SHIFTUP,
		I_SHIFTDOWN,
		INPUTS_LEN
	};
	enum OutputId
	{
		O_CV,
		OUTPUTS_LEN
	};
	enum LightId
	{
		ENUMS(L_SELECTED, NUM_CHANNELS),
		ENUMS(L_ACTIVE, NUM_CHANNELS),
		L_SELECTIONSTART_ERR,
		L_SELECTIONEND_ERR,
		L_PROB_ERR,
		LIGHTS_LEN
	};

	enum SelectionMode
	{
		ENDPOS,
		LENGTH
	};

	int _selectionStart = 0;
	int _selectionEnd = 0;
	int _selectionLength = 0;
	int _outputs[NUM_CHANNELS];
	int _[NUM_CHANNELS];
	float _prob = 0.f;
	bool _selectionStartErr = false;
	bool _selectionEndErr = false;
	bool _probErr = false;
	dsp::SchmittTrigger _trigTrigger;
	dsp::BooleanTrigger _trigButtonTrigger;
	dsp::BooleanTrigger _shiftUpButtonTrigger;
	dsp::BooleanTrigger _shiftDownButtonTrigger;
	dsp::SchmittTrigger _shiftUpTrigger;
	dsp::SchmittTrigger _shiftDownTrigger;
	dsp::ClockDivider _lowPriority;

	Some3()
	{
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configButton(P_TRIGBUTTON, "Manual Trigger");
		configButton(P_SHIFTUPBUTTON, "Shift Up");
		configButton(P_SHIFTDOWNBUTTON, "Shift Down");
		configParam(P_SELECTIONSTART, 0.f, NUM_CHANNELS, 0, "Selection Start");
		paramQuantities[P_SELECTIONSTART]->snapEnabled = true;
		configParam(P_SELECTIONEND, 0.f, NUM_CHANNELS, NUM_CHANNELS, "Selection End");
		paramQuantities[P_SELECTIONEND]->snapEnabled = true;
		configParam(P_PROB, 0.f, 1.f, 0.5f, "Probabilty");
		configInput(I_TRIG, "Trigger CV");
		configInput(I_PROB, "Probability CV");
		configInput(I_SELECTIONSTART, "Selection Start CV");
		configInput(I_SELECTIONEND, "Selection End CV");
		configInput(I_CV, "Input CV");
		configInput(I_SEED, "Seed CV");
		configInput(I_SHIFTUP, "Shift Selection Up");
		configInput(I_SHIFTDOWN, "Shift Selection Down");
		configOutput(O_CV, "CV Out 1");
		_lowPriority.setDivision(16);
	}

	void process(const ProcessArgs &args) override
	{
		_selectionStartErr=false;
		_selectionEndErr=false;
		_probErr=false;
		if (inputs[I_PROB].isConnected())
		{
			_prob = inputs[I_PROB].getVoltage();
			if(_prob<0 || _prob>1)
			{
				_probErr=true;
			}
			_prob = clamp(_prob, 0.f, 1.f);
		}
		else
		{
			_prob = params[P_PROB].getValue();
		}

		// use select start input if connected
		if (inputs[I_SELECTIONSTART].isConnected())
		{
			_selectionStart = inputs[I_SELECTIONSTART].getVoltage();
			_selectionStartErr = _selectionStart < 0 || _selectionStart> NUM_CHANNELS;
			_selectionStart = clamp(_selectionStart, 0, NUM_CHANNELS);
		}
		else
		{
			_selectionStart = params[P_SELECTIONSTART].getValue();
		}

		// use select end input if connected
		if (inputs[I_SELECTIONEND].isConnected())
		{
			_selectionEnd = inputs[I_SELECTIONEND].getVoltage();
			_selectionEndErr = _selectionEnd < 0 || _selectionEnd > NUM_CHANNELS;
			_selectionEnd= clamp(_selectionEnd, 0, NUM_CHANNELS);
		}
		else
		{
			_selectionEnd = params[P_SELECTIONEND].getValue();
		}

		// check out of bound errors
		if(_selectionStart > _selectionEnd)
		{
			_selectionStartErr = true;
			_selectionEndErr = true;
		}

		if (_lowPriority.process())
		{
			// update lights to show selection and active outputs
			for (int i = 0; i < NUM_CHANNELS; i++)
			{
				float v = (i >= _selectionStart && i < _selectionEnd) ? 1 : 0;
				lights[L_SELECTED + i].setBrightness(v);
				lights[L_ACTIVE + i].setBrightness(_outputs[i] > 0);
			}

			// update  error lights
			lights[L_SELECTIONSTART_ERR].setBrightness((_selectionStartErr) ? 1 : 0);
			lights[L_SELECTIONEND_ERR].setBrightness((_selectionEndErr) ? 1 : 0);
			lights[L_PROB_ERR].setBrightness((_probErr) ? 1 : 0);
		}

		// check if shift up button pressed
		// move the selection, but don't change the range
		// upda et the paran knob
		bool shiftUpButtonPressed = _shiftUpButtonTrigger.process(inputs[I_SHIFTUP].getVoltage());
		bool shiftDownButtonPressed = _shiftDownButtonTrigger.process(inputs[I_SHIFTDOWN].getVoltage());
		bool shiftupTriggered = _shiftUpTrigger.process(inputs[I_SHIFTUP].getVoltage());
		bool shiftdownTriggered = _shiftDownTrigger.process(inputs[I_SHIFTDOWN].getVoltage());

		int change = 0;
		if (shiftUpButtonPressed || shiftupTriggered)
		{
			if (_selectionStart > 0)
				change = -1;
		}
		if (shiftDownButtonPressed || shiftdownTriggered)
		{
			if (_selectionEnd < NUM_CHANNELS)
				change = 1;
		}
		if (change != 0)
		{
			DEBUG("Selection Start: %d", _selectionStart);
			DEBUG("Selection End: %d", _selectionEnd);
			_selectionStart += change;
			_selectionEnd += change;
			DEBUG("Selection Start: %d", _selectionStart);
			DEBUG("Selection End: %d", _selectionEnd);
			params[P_SELECTIONSTART].setValue(_selectionStart - 1);
			params[P_SELECTIONEND].setValue(_selectionEnd - 1);
			DEBUG("Param Start: %f", params[P_SELECTIONSTART].getValue());
			DEBUG("Param End: %f", params[P_SELECTIONEND].getValue());
		}

		// check tigger and trigger button
		bool trigButtonPressed = _trigButtonTrigger.process(params[P_TRIGBUTTON].getValue());
		bool trigTriggered = _trigTrigger.process(inputs[I_TRIG].getVoltage());
		if (trigButtonPressed | trigTriggered)
		{
			_selectionLength = _selectionEnd - _selectionStart;
			outputs[O_CV].setChannels(NUM_CHANNELS);


			DEBUG("Selection Start: %d", _selectionStart);
			DEBUG("Selection End: %d", _selectionEnd);
			DEBUG("Selection Length: %d", _selectionLength);

			DEBUG("Probability: %f", _prob);
			int targetActive = _selectionLength * _prob;
			DEBUG("Target Active: %d", targetActive);

			// init the main array
			for (int i = 0; i < NUM_CHANNELS; i++)
			{
				_outputs[i] = -1;
			}

			// temp array to store potential outputs
			int scope[_selectionLength];
			for (int i = 0; i < _selectionLength; i++)
			{
				scope[i] = i + _selectionStart;
				DEBUG("INIT Scope[%d]: %d", i, scope[i]);
			}

			// shuffle array
			for (int i = 0; i < _selectionLength; i++)
			{
				int j = rand() % _selectionLength;
				DEBUG("Swap %d with %d", i, j);
				std::swap(scope[i], scope[j]);
			}
			DEBUG("UPDATE MAIN ARRAY");
			// use only the needed quantity from the front of scope
			for (int i = 0; i < targetActive; i++)
			{
				_outputs[scope[i]] = scope[i];
				DEBUG("scope[%d] = %d, output[scope[i]]=%d ", i, scope[i], _outputs[scope[i]]);
			}

			// process the output array
			for (int i = 0; i < NUM_CHANNELS; i++)
			{
				DEBUG("Output[%d]: %d", i, _outputs[i]);
				if (_outputs[i] == -1)
				{
					// stop output
					DEBUG("Stop output %d", i);
					outputs[O_CV].setVoltage(0.f, i);
				}
				else
				{
					// start output
					DEBUG("Start output %d", i);
					outputs[O_CV].setVoltage(10.f, i);
				}
			}
		}
	}
};

struct Some3Widget : ModuleWidget
{
	Some3Widget(Some3 *module)
	{
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Some3.svg")));

		float width = 20.32;
		float xOffset = 5;
		float yOffset = 15;
		float lx = width / 4;
		float rx = lx * 3;
		float sx = 10;
		float sy = 10;
		float x = xOffset;
		float y = yOffset;

		addInput(createInputCentered<CoffeeInputPortButton>(mm2px(Vec(lx, y)), module, Some3::I_TRIG));
		addParam(createParamCentered<CoffeeTinyButton>(mm2px(Vec(lx + 3.5, y - 3.5)), module, Some3::P_TRIGBUTTON));

		y += sy;
		addInput(createInputCentered<CoffeeInputPort>(mm2px(Vec(rx, y)), module, Some3::I_SEED));
		addInput(createInputCentered<CoffeeInputPortIndicator>(mm2px(Vec(lx, y)), module, Some3::I_PROB));
		addChild(createLightCentered<SmallLight<RedLight> >(mm2px(Vec(lx + 3.5, y + 3.5)), module, Some3::L_PROB_ERR));

		y += sy;
		addParam(createParamCentered<CoffeeKnob6mm>(mm2px(Vec(lx, y)), module, Some3::P_PROB));

		y = yOffset + sy + sy + sy + 2.5;
		addInput(createInputCentered<CoffeeInputPortIndicator>(mm2px(Vec(lx, y)), module, Some3::I_SELECTIONSTART));
		addChild(createLightCentered<SmallLight<RedLight> >(mm2px(Vec(lx + 3.5, y + 3.5)), module, Some3::L_SELECTIONSTART_ERR));
		y += sy;
		addParam(createParamCentered<CoffeeKnob8mm>(mm2px(Vec(lx, y)), module, Some3::P_SELECTIONSTART));

		y += sy + 2.5;
		addInput(createInputCentered<CoffeeInputPortButton>(mm2px(Vec(lx, y)), module, Some3::I_SHIFTUP));
		addParam(createParamCentered<CoffeeTinyButton>(mm2px(Vec(lx + 3.5, y - 3.5)), module, Some3::P_SHIFTUPBUTTON));
		y += sy;
		addInput(createInputCentered<CoffeeInputPortButton>(mm2px(Vec(lx, y)), module, Some3::I_SHIFTDOWN));
		addParam(createParamCentered<CoffeeTinyButton>(mm2px(Vec(lx + 3.5, y - 3.5)), module, Some3::P_SHIFTDOWNBUTTON));

		y = yOffset + sy * 7;
		y += sy - 2.5;
		addInput(createInputCentered<CoffeeInputPortIndicator>(mm2px(Vec(lx, y)), module, Some3::I_SELECTIONEND));
		addChild(createLightCentered<SmallLight<RedLight> >(mm2px(Vec(lx + 3.5, y + 3.5)), module, Some3::L_SELECTIONEND_ERR));
		y += sy;
		addParam(createParamCentered<CoffeeKnob8mm>(mm2px(Vec(lx, y)), module, Some3::P_SELECTIONEND));

		// input and output
		y = yOffset + (sy * 10);
		addInput(createInputCentered<CoffeeInputPort>(mm2px(Vec(lx, y)), module, Some3::I_CV));
		y = yOffset + (sy * 10);
		addOutput(createOutputCentered<CoffeeOutputPort>(mm2px(Vec(rx, y)), module, Some3::O_CV));

		y = yOffset + sy + sy + sy;
		for (int i = 0; i < NUM_CHANNELS; i++)
		{
			addChild(createLightCentered<MediumLight<OrangeLight> >(mm2px(Vec(rx - 2, y + (i * 4))), module, Some3::L_SELECTED + i));
			addChild(createLightCentered<MediumLight<GreenLight> >(mm2px(Vec(rx + 2, y + (i * 4))), module, Some3::L_ACTIVE + i));
		}
	}
};

Model *modelSome3 = createModel<Some3, Some3Widget>("Some3");