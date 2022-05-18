#include "plugin.hpp"
#define MAX_SIG 8

struct Some : Module {
	enum ParamId {
		P_TRIG1,
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
	bool signals[MAX_SIG]={};

	Some() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(P_TRIG1, 0.f, 1.f, 0.f, "Trigger");
		configParam(P_PROB, 0.f, 1.f, 0.5f, "Probability");
		configInput(I_TRIG, "Trigger");
		configInput(I_PROB, "Probability");
		for(int i=0; i<MAX_SIG; i++){
			configInput(I_V + i, "");
			configOutput(O_V + i, "");
		}
	}

	void process(const ProcessArgs& args) override {
		
		float p_prob=params[P_PROB].getValue();
		if(inputs[I_PROB].isConnected()){
			p_prob=clamp(inputs[I_PROB].getVoltage(),0.f,1.f);
		}
		
		float b_trigger=params[P_TRIG1].getValue();
		float i_trigger=inputs[I_TRIG].getVoltage();

		// find the connected inputs and save to vector array
		std::vector<int>sigs;
		for ( int i = 0; i < MAX_SIG; i++)
		{
			if(inputs[I_V + i].isConnected()) sigs.push_back(i);
		}

		if( clockTrigger.process(i_trigger) | buttonTrigger.process(b_trigger)){

			//shuffle the array
			for(int i=0;i<int(sigs.size());i++){
				std::swap(sigs[i], sigs[sigs.size() * random::uniform()]);
			}

			//reset the main array
			for(int i=0; i<MAX_SIG; i++) signals[i]=false;
			//flip the selected channels from the vector arracy
			int j=sigs.size() * p_prob;
			for(int i=0; i<j; i++) signals[sigs[i]]=true; 

			for(int i=0; i<MAX_SIG; i++){
				if(signals[i]){
					lights[L_LED + i].setBrightness(1);
					outputs[O_V + i].setVoltage( inputs[I_V + i].getVoltage() );
				} else {
					lights[L_LED + i].setBrightness(0);
					outputs[O_V + i].setVoltage( 0.f );
				}
			}
		}
	}
};

struct SomeWidget : ModuleWidget {
	SomeWidget(Some* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Some.svg")));
		float lx=30.48/4;
		float rx=30.48-lx;
		float mx=30.48/2;

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(lx, 15)), module, Some::I_TRIG));
		addParam(createParamCentered<VCVButton>(mm2px(Vec(rx, 15)), module, Some::P_TRIG1));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(lx, 30)), module, Some::I_PROB));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(rx, 30)), module, Some::P_PROB));

		for(int i=0; i<MAX_SIG; i++){
			float y=112-((7-i)*10);
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(lx, y)), module, Some::I_V + i ));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(rx, y)), module, Some::O_V + i ));
			addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(mx, y)), module, Some::L_LED + i));
		}
	}
};


Model* modelSome = createModel<Some, SomeWidget>("Some");