#include "plugin.hpp"
#include "components.hpp"
#define NUM_GROUPS 3

struct Tap : Module {
	enum ParamId {
		ENUMS(P_BUTTON, NUM_GROUPS),
		PARAMS_LEN
	};
	enum InputId {
		INPUTS_LEN
	};
	enum OutputId {
		ENUMS(O_TRIG, NUM_GROUPS),
		ENUMS(O_GATE, NUM_GROUPS),
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	dsp::BooleanTrigger _buttonTrigger[NUM_GROUPS];
	dsp::PulseGenerator _trigPulse[NUM_GROUPS];

	Tap() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configButton(P_BUTTON, "Button");
		for(int i = 0; i < NUM_GROUPS; i++) {
			configOutput(O_TRIG + i, string::f("Trig %d", i + 1));
			configOutput(O_GATE + i, string::f("Gate %d", i + 1));
		}
	}

	void process(const ProcessArgs& args) override {
		for(int i=0; i<NUM_GROUPS; i++) {
			if(_buttonTrigger[i].process(params[P_BUTTON + i].getValue())) {
				_trigPulse[i].trigger(1e-3);
				outputs[O_TRIG + i].setVoltage(10.f);
			} 

			outputs[O_GATE + i].setVoltage( (params[P_BUTTON + i].getValue() > 0.5f) ? 10.f : 0.f);

			if(!_trigPulse[i].process(args.sampleTime)) {
				outputs[O_TRIG + i].setVoltage(0.f);
			}
		}

	}
};


struct TapWidget : ModuleWidget {
	TapWidget(Tap* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Tap.svg")));
		
		float yOffset = 17.50;
		float gy = 35;
		float x,y;
		x = 10.16/2;
		y=yOffset;

		for(int i = 0; i < NUM_GROUPS; i++) {
			y = yOffset + (i * gy);
			addParam(createParamCentered<CoffeeInputButton5mm>(mm2px(Vec(x, y)), module, Tap::P_BUTTON + i ));
			addOutput(createOutputCentered < CoffeeOutputPort > (mm2px(Vec(x, y + 10)), module, Tap::O_GATE + i));
			addOutput(createOutputCentered < CoffeeOutputPort > (mm2px(Vec(x, y + 20)), module, Tap::O_TRIG + i));
		}
	}
};


Model* modelTap = createModel<Tap, TapWidget>("Tap");