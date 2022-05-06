#include "plugin.hpp"


struct Some : Module {
	enum ParamId {
		P_TRIG1,
		P_PROB,
		PARAMS_LEN
	};
	enum InputId {
		I_TRIG,
		I_PROB,
		ENUMS(I_V,8),
		INPUTS_LEN
	};
	enum OutputId {
		ENUMS(O_V,8),
		OUTPUTS_LEN
	};
	enum LightId {
		L_LED,
		LIGHTS_LEN
	};

	Some() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(P_TRIG1, 0.f, 1.f, 0.f, "");
		configParam(P_PROB, 0.f, 1.f, 0.f, "");
		configInput(I_TRIG, "");
		configInput(I_PROB, "");
		for(int i=0; i<8; i++){
			configInput(I_V + i, "");
			configOutput(O_V + i, "");
		}
	}

	void process(const ProcessArgs& args) override {
	}
};


struct SomeWidget : ModuleWidget {
	SomeWidget(Some* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Some.svg")));

		addParam(createParamCentered<Trimpot>(mm2px(Vec(22.103, 12.912)), module, Some::P_TRIG1));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(22.103, 27.299)), module, Some::P_PROB));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.535, 12.912)), module, Some::I_TRIG));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.535, 27.299)), module, Some::I_PROB));

		for(int i=0; i<8; i++){
			float y=40+(i*10);
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.535, y)), module, Some::I_V + i ));
		}
			// addOutput(createInputCentered<PJ301MPort>(mm2px(Vec(38.42-8.535, 40)), module, Some::O_V + 0 ));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(15.319, 27.299)), module, Some::L_LED));
	}
};


Model* modelSome = createModel<Some, SomeWidget>("Some");