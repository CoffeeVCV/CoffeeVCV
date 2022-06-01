#include "plugin.hpp"


struct Test : Module {
	enum ParamId {
		P_P_TRIG,
		PARAMS_LEN
	};
	enum InputId {
		I_I_TRIG,
		INPUTS_LEN
	};
	enum OutputId {
		O_O_CV1,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	Test() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(P_P_TRIG, 0.f, 1.f, 0.f, "");
		configInput(I_I_TRIG, "");
		configOutput(O_O_CV1, "");
	}

	void process(const ProcessArgs& args) override {
	}
};


struct TestWidget : ModuleWidget {
	TestWidget(Test* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/test.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.752, 59.705)), module, Test::P_P_TRIG));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.752, 24.986)), module, Test::I_I_TRIG));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.752, 94.423)), module, Test::O_O_CV1));
	}
};


Model* modelTest = createModel<Test, TestWidget>("test");