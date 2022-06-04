#include "plugin.hpp"
#include "components.hpp"
#define NUM_SETS 2

struct Liken : Module
{
	enum ParamId
	{
		P_TRIGBUTTON,
		P_THRESHOLD,
		ENUMS(P_SCALE, NUM_SETS),
		ENUMS(P_OFFSET, NUM_SETS),
		ENUMS(P_MODSWITCH, NUM_SETS),
		P_TRACKHOLDSWITCH,
		PARAMS_LEN
	};
	enum InputId
	{
		I_TRIG,
		I_THRESHOLD,
		ENUMS(I_CV, NUM_SETS),
		I_SOURCECV,
		INPUTS_LEN
	};
	enum OutputId
	{
		O_CV,
		ENUMS(O_TRIG, NUM_SETS),
		OUTPUTS_LEN
	};
	enum LightId
	{
		ENUMS(L_Active, NUM_SETS),
		LIGHTS_LEN
	};

	enum AB
	{
		A = 1,
		B = 2
	};
	enum TRACKHOLD
	{
		TRACK,
		HOLD
	};
	enum MODREPLACE
	{
		MODIFY,
		REPLACE
	};
	dsp::ClockDivider _lowPriority;
	dsp::SchmittTrigger _sampleTrigger;
	dsp::BooleanTrigger _buttonTrigger;
	dsp::PulseGenerator _trigPulse[NUM_SETS];
	bool ready = true;
	float sample = 0.f;
	float lastsample = 0.f;
	float lastset = -1;

	Liken()
	{
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configButton(P_TRIGBUTTON, "Manual trigger");
		configParam(P_THRESHOLD, -10.f, 10.f, 0.f, "Threshold");
		for (int i = 0; i < NUM_SETS; i++)
		{
			configParam(P_SCALE + i, -10.f, 10.f, 1.f, string::f("Scale %d", i + 1));
			configParam(P_OFFSET + i, -10.f, 10.f, 0.f, string::f("Offset %d", i + 1));
			configSwitch(P_MODSWITCH + i, 0, 1, 0, string::f("Modify/Replace %d", i + 1), {"Modify", "Replace"});
			configInput(I_CV + i, string::f("CV %d", i + 1));
			configOutput(O_TRIG + i, string::f("Triger %d", i + 1));
		}

		configSwitch(P_TRACKHOLDSWITCH, 0, 1, 0, "Track or Hold", {"Track", "Hold"});
		configInput(I_TRIG, "Trigger");
		configInput(I_CV, "Source CV");
		configInput(I_THRESHOLD, "Threshold CV");
		configOutput(O_CV, "CV");
		_lowPriority.setDivision(32);
	}

	float calculate_output(float sample, float input, float scale, float offset, bool mode)
	{
		float output = 0.f;
		if (mode == MODIFY)
		{
			output = (sample + input + offset) * scale;
		}
		else if (mode == REPLACE)
		{
			output = (input + offset) * scale;
		}
		return output;
	}

	void process(const ProcessArgs &args) override
	{
		// check trigger input and trigger button
		bool triggered = _sampleTrigger.process(inputs[I_TRIG].getVoltage());
		bool trigButtonPressed = _buttonTrigger.process(params[P_TRIGBUTTON].getValue());
		float modInV[NUM_SETS];

		for (int i = 0; i < NUM_SETS; i++)
		{
			bool pulse = _trigPulse[i].process(args.sampleTime);
			outputs[O_TRIG + i].setVoltage(pulse ? 10.f : 0.f);
			modInV[i] = inputs[I_CV + i].getVoltage();
		}

		// check if sample or hold
		bool track = params[P_TRACKHOLDSWITCH].getValue() > 0.5f;

		// if hold and triggerd take a sample
		if (track == HOLD)
		{
			if (triggered || trigButtonPressed)
			{
				ready = false;
				sample = inputs[I_SOURCECV].getVoltage();
			}
		}
		else
		{ // just take a sample
			sample = inputs[I_SOURCECV].getVoltage();
		}
		ready = !_sampleTrigger.isHigh();

		if (sample != lastsample)
		{
			// lastsample=sample;
			// if threshold input is connect use it otherwise use the param
			float threshold = inputs[I_THRESHOLD].isConnected() ? inputs[I_THRESHOLD].getVoltage() : params[P_THRESHOLD].getValue();
			// check if sample is higher or lower then threshold
			// to determine which set of params to use for output
			int set = (sample > threshold) ? 0 : 1;
			float result = 0;

			// calculate output
			result = calculate_output(sample, modInV[set], params[P_SCALE + set].getValue(), params[P_OFFSET + set].getValue(), params[P_MODSWITCH + set].getValue());
			int i = (set + 1) % 2;
			outputs[O_CV].setVoltage(result);
			if (lastset != set)
			{
				lastset = set;
				lights[L_Active + set].setBrightness(1.f);
				lights[L_Active + i].setBrightness(0.f);
				_trigPulse[set].trigger(1e-3f);
				outputs[O_TRIG + set].setVoltage(10.f);
			}
		}
	}
};

struct LikenWidget : ModuleWidget
{
	LikenWidget(Liken *module)
	{
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Liken.svg")));

		float yOffset = 12.f;
		float sy = 10;
		float width = 20.32;
		float rx = width / 4;
		float mx = width / 2;
		float lx = rx * 3;
		float x = mx;
		float y = yOffset;
		// input trigger
		addInput(createInputCentered<CoffeeInputPortButton>(mm2px(Vec(x, y)), module, Liken::I_TRIG));
		addParam(createParamCentered<CoffeeTinyButton>(mm2px(Vec(x + 3.5, y - 3.5)), module, Liken::P_TRIGBUTTON));

		// track or hold switch
		y += sy - 2;
		addParam(createParamCentered<CoffeeSwitch2PosHori>(mm2px(Vec(x, y)), module, Liken::P_TRACKHOLDSWITCH));

		// input source
		y += sy + 5;
		addInput(createInputCentered<CoffeeInputPort>(mm2px(Vec(x, y)), module, Liken::I_SOURCECV));

		// threshold input and param
		y += sy + sy - 5;
		addInput(createInputCentered<CoffeeInputPort>(mm2px(Vec(rx, y)), module, Liken::I_THRESHOLD));
		addParam(createParamCentered<CoffeeKnob8mm>(mm2px(Vec(lx, y)), module, Liken::P_THRESHOLD));

		y += (sy * 2) - 5;
		for (int i = 0; i < NUM_SETS; i++)
		{
			// Raplacement A input, offset, scale and switch
			x = (i == 0) ? rx : lx;
			addChild(createLightCentered<MediumLight<OrangeLight> >(mm2px(Vec(x, y - 7.5)), module, Liken::L_Active + i));
			addInput(createInputCentered<CoffeeInputPort>(mm2px(Vec(x, y)), module, Liken::I_CV + i));
			addParam(createParamCentered<CoffeeSwitch2PosHori>(mm2px(Vec(x, y + sy - 2)), module, Liken::P_MODSWITCH + i));
			addParam(createParamCentered<CoffeeKnob6mm>(mm2px(Vec(x, y + sy + sy - 5)), module, Liken::P_OFFSET + i));
			addParam(createParamCentered<CoffeeKnob6mm>(mm2px(Vec(x, y + sy + sy + 3)), module, Liken::P_SCALE + i));
			addOutput(createOutputCentered<CoffeeOutputPort>(mm2px(Vec(x, y + sy + sy + sy + 2)), module, Liken::O_TRIG + i));
		}

		// output
		x = mx;
		y = yOffset + (sy * 10);
		addOutput(createOutputCentered<CoffeeOutputPort>(mm2px(Vec(x, y)), module, Liken::O_CV));
	}
};

Model *modelLiken = createModel<Liken, LikenWidget>("Liken");