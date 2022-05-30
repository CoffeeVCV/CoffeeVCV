#include "plugin.hpp"
#include "components.hpp"

struct HiLo : Module {
	enum ParamId {
		P_TRIG,
		P_OFFSET1,
		P_SCALE1,
		P_OFFSET2,
		P_SCALE2,
		PARAMS_LEN
	};
	enum InputId {
		I_TRIG,
		I_V1,
		I_V2,
		INPUTS_LEN
	};
	enum OutputId {
		O_HI,
		O_LO,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	HiLo() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configButton(P_TRIG, "Trigger");
		configParam(P_OFFSET1, -5.0f, 5.0f, 0.0f, "Offset", " V");
		configParam(P_SCALE1, -5.0f, 5.0f, 1.0f, "Scale");
		configParam(P_SCALE2, -5.0f, 5.0f, 1.0f, "Scale");
		configParam(P_OFFSET2, -5.0f, 5.0f, 0.0f, "Offset", " V");
		configInput(I_TRIG, "Trig");
		configInput(I_V1, "Input1");
		configInput(I_V2, "Input2");
		configOutput(O_HI, "High");
		configOutput(O_LO, "Low");
	}
	
    dsp::SchmittTrigger clockTrigger;
	dsp::BooleanTrigger buttonTrigger;
	bool track=false;

	void process(const ProcessArgs& args) override {

		float p_scale1=params[P_SCALE1].getValue();
		float p_offset1=params[P_OFFSET1].getValue();
		float p_scale2=params[P_SCALE2].getValue();
		float p_offset2=params[P_OFFSET2].getValue();

		float b_trigger=params[P_TRIG].getValue();
		float i_trigger=inputs[I_TRIG].getVoltage();

		float v1=0;
		if(inputs[I_V1].isConnected())
			v1=inputs[I_V1].getVoltage();
		v1=(v1+p_offset1)*p_scale1;

		float v2=0;
		if(inputs[I_V2].isConnected())
			v2=inputs[I_V2].getVoltage();
		v2=(v2+p_offset2)*p_scale2;

		if( (track and !inputs[I_TRIG].isConnected()) | clockTrigger.process(i_trigger) | buttonTrigger.process(b_trigger)){
			outputs[O_HI].setVoltage(fmax(v1,v2));
			outputs[O_LO].setVoltage(fmin(v1,v2));
		}
	}
};

struct HiLoWidget : ModuleWidget {
	HiLoWidget(HiLo* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/HiLo.svg")));
		float mx=10.16/2;
		float y=15;
		addInput(createInputCentered<CoffeeInputPortButton>(mm2px(Vec(mx, y)), module, HiLo::I_TRIG));
		addParam(createParamCentered<CoffeeTinyButton>(mm2px(Vec(mx+3.5, y-3.5)), module, HiLo::P_TRIG));

		//Inputs
		y=30;
		addInput(createInputCentered<CoffeeInputPort>(mm2px(Vec(mx, y)), module, HiLo::I_V1));
		addParam(createParamCentered<CoffeeKnob6mm>(mm2px(Vec(mx, y+10)), module, HiLo::P_OFFSET1));
		addParam(createParamCentered<CoffeeKnob6mm>(mm2px(Vec(mx, y+20)), module, HiLo::P_SCALE1));
		
		y=61;
		addInput(createInputCentered<CoffeeInputPort>(mm2px(Vec(mx, y)), module, HiLo::I_V2));
		addParam(createParamCentered<CoffeeKnob6mm>(mm2px(Vec(mx, y+10)), module, HiLo::P_OFFSET2));		
		addParam(createParamCentered<CoffeeKnob6mm>(mm2px(Vec(mx, y+20)), module, HiLo::P_SCALE2));

		//Outputs
		addOutput(createOutputCentered<CoffeeOutputPort>(mm2px(Vec(mx, 102)), module, HiLo::O_HI));
		addOutput(createOutputCentered<CoffeeOutputPort>(mm2px(Vec(mx, 112)), module, HiLo::O_LO));
	}

	void appendContextMenu(Menu* menu) override {
		HiLo* module = dynamic_cast<HiLo*>(this->module);
		assert(module);

		menu->addChild(new MenuSeparator());
		menu->addChild(createSubmenuItem("Track/Hold Inputs", "", [=](Menu* menu) {
			Menu* trackholdMenu = new Menu();
			trackholdMenu->addChild(createMenuItem("Track", CHECKMARK(module->track == true), [module]() { module->track = true; }));
			trackholdMenu->addChild(createMenuItem("Hold", CHECKMARK(module->track == false), [module]() { module->track = false; }));
			menu->addChild(trackholdMenu);
		}));
	}	
};


Model* modelHiLo = createModel<HiLo, HiLoWidget>("HiLo");