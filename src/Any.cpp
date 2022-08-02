#include "plugin.hpp"
#include "components.hpp"
#define NUM_ROWS 8

struct Any : Module
{
	enum ParamId
	{
		P_AND_SWITCH,
		PARAMS_LEN
	};
	enum InputId
	{
		ENUMS(I_TRIG, NUM_ROWS),
		INPUTS_LEN
	};
	enum OutputId
	{
		O_TRIG,
		OUTPUTS_LEN
	};
	enum LightId
	{
		LIGHTS_LEN
	};

	enum Mode
	{
		ORMODE,
		ANDMODE
	};

	dsp::SchmittTrigger _rowTrigger[NUM_ROWS];
	dsp::PulseGenerator _outPulse;
	bool _ready[NUM_ROWS];
	int _triggered = 0;
	int _connected = 0;

	Any()
	{
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configSwitch(P_AND_SWITCH, 0, 1, 1, "Mode", {"AND", "OR"});
		for (int i = 0; i < NUM_ROWS; i++)
		{
			configInput(I_TRIG + i, string::f("Trigger %d", i));
		}
		configOutput(O_TRIG, "");
	}

	void process(const ProcessArgs &args) override
	{
		int mode = params[P_AND_SWITCH].getValue();
		if (!_outPulse.process(args.sampleTime))
		{
			outputs[O_TRIG].setVoltage(0.);
		}
		_connected = 0;
		_triggered = 0;
		for (int i = 0; i < NUM_ROWS; i++)
		{
			if (inputs[I_TRIG + i].isConnected())
			{
				_connected++;
				bool triggered = _rowTrigger[i].process(inputs[I_TRIG + i].getVoltage());
				if (_ready[i] && triggered)
				{
					_triggered += 1;
					_ready[i] = false;
				}
			}
			_ready[i] = !_rowTrigger[i].isHigh();
		}

		if ((mode == ORMODE && _triggered > 0) || (mode == ANDMODE && _triggered == _connected))
		{
			outputs[O_TRIG].setVoltage(10.f);
			_outPulse.trigger(1e-3);
		}
	}
};

struct AnyWidget : ModuleWidget
{
	AnyWidget(Any *module)
	{
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Any.svg")));

		float yOffset = 16.f;
		float sy = 10.f;
		float width = 10.16;
		float mx = width / 2;
		float y = yOffset;

		addParam(createParamCentered<CoffeeSwitch2PosHori>(mm2px(Vec(mx, y)), module, Any::P_AND_SWITCH));
		y += sy;
		for (int i = 0; i < NUM_ROWS; i++)
		{
			addInput(createInputCentered<CoffeeInputPort>(mm2px(Vec(mx, y)), module, Any::I_TRIG + i));
			y += sy;
		}
		y += 6;
		addOutput(createOutputCentered<CoffeeOutputPort>(mm2px(Vec(mx, y)), module, Any::O_TRIG));
	}
};

Model *modelAny = createModel<Any, AnyWidget>("Any");