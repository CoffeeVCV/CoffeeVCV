#include "plugin.hpp"
#define MAX_SIG 8

struct Some : Module {
	enum ParamId {
		P_TRIG,
		P_PROB,
		PARAMS_LEN
	};
	enum InputId {
		I_TRIG,
		I_PROB,
		ENUMS(I_V,MAX_SIG),
		INPUTS_LEN
	};
	enum OutputId {
		ENUMS(O_V,MAX_SIG),
		OUTPUTS_LEN
	};
	enum LightId {
		ENUMS(L_LED,MAX_SIG),
		LIGHTS_LEN
	};

	dsp::SchmittTrigger clockTrigger;
	dsp::BooleanTrigger buttonTrigger;

	Some() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(P_TRIG, 0.f, 1.f, 0.f, "Trigger");
		configParam(P_PROB, 0.f, 1.f, 0.5f, "Probability");
		configInput(I_TRIG, "Trigger");
		configInput(I_PROB, "Probability");
		for(int i = 0; i < MAX_SIG; i++) {
			std::string index_str = string::f("#%d", i + 1);
			configInput(I_V + i, index_str);
			configOutput(O_V + i, index_str);
		}
	}

	void process(const ProcessArgs& args) override {
		bool button_pressed = buttonTrigger.process(params[P_TRIG].getValue());
		bool clock_ticked = clockTrigger.process(inputs[I_TRIG].getVoltage());

		if (!button_pressed && !clock_ticked) return;

		float p_prob;
		if (inputs[I_PROB].isConnected()) {
			p_prob = clamp(inputs[I_PROB].getVoltage(), 0.f, 1.f);
		} else {
			p_prob = params[P_PROB].getValue();
		}
		// find the connected inputs and randomly distribute at the start of an array
		// unconnected inputs will be added to the back of the array to be zeroed
		int sigs[MAX_SIG];
		int num_sigs = 0;
		int unused_sig_index = MAX_SIG;
		for (int i = 0; i < MAX_SIG; i++) {
			if (inputs[I_V + i].isConnected()) {
				if (num_sigs++) {
					// add an item to a random array, maintaining random distrobution
					int swap_i = num_sigs * random::uniform();
					sigs[num_sigs - 1] = sigs[swap_i];
					sigs[swap_i] = i;
				} else {
					// create a "random" array of length 1
					sigs[0] = i;
				}
			} else {
				sigs[--unused_sig_index] = i;
			}
		}
		// only enable a portion of the connected channels, then zero the rest
		int num_enabled = num_sigs * p_prob;
		for (int i = 0; i < MAX_SIG; i++) {
			int j = sigs[i];
			if (i < num_enabled) {
				lights[L_LED + j].setBrightness(1.f);
				outputs[O_V + j].setVoltage(inputs[I_V + j].getVoltage());
			} else {
				lights[L_LED + j].setBrightness(0.f);
				outputs[O_V + j].setVoltage(0.f);
			}
		}
	}
};

struct SomeWidget : ModuleWidget {
	SomeWidget(Some* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Some.svg")));
		float lx = 30.48 / 4;
		float rx = 30.48 - lx;
		float mx = 30.48 / 2;

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(lx, 15)), module, Some::I_TRIG));
		addParam(createParamCentered<VCVButton>(mm2px(Vec(rx, 15)), module, Some::P_TRIG));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(lx, 30)), module, Some::I_PROB));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(rx, 30)), module, Some::P_PROB));

		for(int i = 0; i < MAX_SIG; i++) {
			float y = 42 + 10 * i;
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(lx, y)), module, Some::I_V + i));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(rx, y)), module, Some::O_V + i));
			addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(mx, y)), module, Some::L_LED + i));
		}
	}
};


Model* modelSome = createModel<Some, SomeWidget>("Some");
