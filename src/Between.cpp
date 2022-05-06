#include "plugin.hpp"

struct Between : Module {
	enum ParamId {
		P_TRIG,
		P_CV1,
		P_CV2,
		P_OFFSET1,
		P_OFFSET2,
		PARAMS_LEN
	};
	enum InputId {
		I_TRIG,
		I_CV1,
		I_CV2,
		INPUTS_LEN
	};
	enum OutputId {
		O_CV1,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};
	
	float lastMaxCV = 0.f;
	float lastMinCV = 10.f;
	float hold = 0.f;

	dsp::SchmittTrigger clockTrigger;
	dsp::BooleanTrigger buttonTrigger;

	Between() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configButton(P_TRIG, "Trigger");
		configParam(P_CV1, -5.f, 5.f, 0.f, "CV1");
		configParam(P_CV2, -5.f, 5.f, 0.f, "CV2");		
		configParam(P_OFFSET1, -5.f, 5.f, 0.f, "Offset1");
		configParam(P_OFFSET2, -5.f, 5.f, 0.f, "Offset2");
		configInput(I_TRIG, "Trigger");
		configInput(I_CV1, "CV1");
		configInput(I_CV2, "CV2");
		configOutput(O_CV1, "Out");
	}

	void process(const ProcessArgs& args) override {
		// no output, no point
		if(!outputs[O_CV1].isConnected()) { return; }

		float CV1 = params[P_CV1].getValue();
		if(inputs[I_CV1].isConnected()){
			CV1=inputs[I_CV1].getVoltage();
		}

		float CV2 = params[P_CV2].getValue();
		if(inputs[I_CV2].isConnected()){
			CV2=inputs[I_CV2].getVoltage();
		}

		float b_trigger=params[P_TRIG].getValue();
		float i_trigger=inputs[I_TRIG].getVoltage();
		if( (clockTrigger.process(i_trigger) | buttonTrigger.process(b_trigger))){
			CV1+=params[P_OFFSET1].getValue();
			CV2+=params[P_OFFSET2].getValue();
			if(CV1<CV2) {
				hold = ((CV1-CV2) * random::uniform()) + CV2;
			} else {
				hold = ((CV2-CV1) * random::uniform()) + CV1;
			}
		} 
		
		outputs[O_CV1].setVoltage(hold);

	}
};

struct BetweenWidget : ModuleWidget {
	BetweenWidget(Between* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Between.svg")));
	
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 15)), module, Between::I_TRIG));
		addParam(createParamCentered<VCVButton>(mm2px(Vec(7.62, 25)), module, Between::P_TRIG));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 42)), module, Between::I_CV1));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(7.62, 52)), module, Between::P_CV1));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(7.62, 62)), module, Between::P_OFFSET1));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 78)), module, Between::I_CV2));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(7.62, 88)), module, Between::P_CV2));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(7.62, 98)), module, Between::P_OFFSET2));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 112)), module, Between::O_CV1));
	}
};

Model* modelBetween = createModel<Between, BetweenWidget>("Between");