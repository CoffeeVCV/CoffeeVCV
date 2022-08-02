#include "plugin.hpp"
#include "components.hpp"
#define NUM_ROWS 8

struct Any : Module {
	enum ParamId {
		PARAMS_LEN
	};
	enum InputId {
		ENUMS(I_TRIG,NUM_ROWS),
		INPUTS_LEN
	};
	enum OutputId {
		O_TRIG,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	dsp::SchmittTrigger _rowTrigger[NUM_ROWS];
	dsp::PulseGenerator _outPulse;
	bool _ready[NUM_ROWS];
	bool _triggered=false;

	Any() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		for(int i=0; i<NUM_ROWS; i++){		
			configInput(I_TRIG+i, string::f("Trigger %d", i));
		}
		configOutput(O_TRIG, "");
	}

	void process(const ProcessArgs& args) override {
		if(!_outPulse.process(args.sampleTime)){
			outputs[O_TRIG].setVoltage(0.);
		}		

		_triggered=false;
		for(int i=0; i<NUM_ROWS; i++){
			bool triggered=_rowTrigger[i].process(inputs[I_TRIG+i].getVoltage());
			//bool triggered=inputs[I_TRIG+i].getVoltage()>0.5 ? true : false;
			if(_ready[i] && triggered){
			//if(triggered){
				_triggered=true;
				_ready[i]=false;
			}
			_ready[i]=!_rowTrigger[i].isHigh();
		}

		if(_triggered){
			outputs[O_TRIG].setVoltage(10.f);
			_outPulse.trigger(1e-3);
		}

	}
};


struct AnyWidget : ModuleWidget {
	AnyWidget(Any* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Any.svg")));

		float yOffset = 26.f;
		float sy = 10.f;
		float width=10.16;
		float mx=width/2;
		float y=yOffset;

		for(int i=0; i<NUM_ROWS; i++){		
			addInput(createInputCentered<CoffeeInputPort>(mm2px(Vec(mx, y)), module, Any::I_TRIG+i));
			y+=sy;
		}
		addOutput(createOutputCentered<CoffeeOutputPort>(mm2px(Vec(mx, y)), module, Any::O_TRIG));
	}
};


Model* modelAny = createModel<Any, AnyWidget>("Any");