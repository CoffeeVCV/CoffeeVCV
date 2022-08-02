#include "plugin.hpp"
#include "JWModules/QuantizeUtils.cpp"
#include "components.hpp"

struct Quant : Module, QuantizeUtils {
	enum ParamId {
		P_ROOTNOTE,
		P_SCALE,
		P_OCTAVE,
		PARAMS_LEN
	};
	enum InputId {
		I_ROOTNOTE,
		I_SCALE,
		I_OCTAVE,
		I_VOCT,
		INPUTS_LEN
	};
	enum OutputId {
		O_VOCT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};
	float _rootNote=-1;
	float _scale=-1;
	float _octave=-1;
	float _lastRootNote=-1;
	float _lastScale=-1;
	float _lastOctave=-1;

	Quant() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(P_ROOTNOTE, 0.0, QuantizeUtils::NUM_NOTES-1, QuantizeUtils::NOTE_C, "Root Note");
		paramQuantities[P_ROOTNOTE]->displayOffset = 1;
		paramQuantities[P_ROOTNOTE]->snapEnabled = true;

		configParam(P_SCALE, 0.0, QuantizeUtils::NUM_SCALES-1, QuantizeUtils::MINOR, "Scale");
		paramQuantities[P_SCALE]->displayOffset = 1;
		paramQuantities[P_SCALE]->snapEnabled = true;

		configParam(P_OCTAVE, -5.f, 5.f, 0.f, "Octave");
		paramQuantities[P_OCTAVE]->snapEnabled = true;

		configInput(I_ROOTNOTE, "Root Note");
		configInput(I_SCALE, "Scale");
		configInput(I_OCTAVE, "Octave");
		configInput(I_VOCT, "V/OCT In");
		configOutput(O_VOCT, "V/OCT Out");
	}

	void process(const ProcessArgs& args) override {

		if(inputs[I_ROOTNOTE].isConnected()) {
			_rootNote = clamp((int)inputs[I_ROOTNOTE].getVoltage(),0,QuantizeUtils::NUM_NOTES-1);
		} else {
			_rootNote = params[P_ROOTNOTE].getValue();
		}
		if(_rootNote != _lastRootNote) {
			_lastRootNote = _rootNote;
			paramQuantities[P_ROOTNOTE]->description = noteName((int)_rootNote);
		}

		if(inputs[I_SCALE].isConnected()) {
			_scale = clamp((int)inputs[I_SCALE].getVoltage(),0,QuantizeUtils::NUM_SCALES-1);
		} else {
			_scale = params[P_SCALE].getValue();
		}
		if(_scale != _lastScale) {
			_lastScale = _scale;
			paramQuantities[P_SCALE]->description = scaleName((int)_scale);
		}

		if(inputs[I_OCTAVE].isConnected()) {
			_octave = clamp((int)inputs[I_OCTAVE].getVoltage(),-5,5);
		} else {
			_octave = params[P_OCTAVE].getValue();
		}

		if(inputs[I_VOCT].isConnected()) {
			float v=closestVoltageInScale(inputs[I_VOCT].getVoltage(), (int)_rootNote, (int)_scale);
			//float v=0.f;
			outputs[O_VOCT].setVoltage(v+_octave);
		}
	}
};

struct NoteKnob : CoffeeKnob8mm {
	QuantizeUtils *quantizeUtils;
	NoteKnob(){
		snap = true;
	}
	std::string formatCurrentValue()  {
		if(getParamQuantity() != NULL){
			return quantizeUtils->noteName(int(getParamQuantity()->getDisplayValue()));
		}
		return "";
	}
};

struct QuantWidget : ModuleWidget {
	QuantWidget(Quant* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Quant.svg")));

		float yOffset = 15.0;
		float sy=10;
		float width = 10.16;
		float mx =  width/2;
		float y = yOffset;

		addParam(createParamCentered<CoffeeKnob8mm>(mm2px(Vec(mx, y)), module, Quant::P_ROOTNOTE));
		addInput(createInputCentered<CoffeeInputPort>(mm2px(Vec(mx, y+sy)), module, Quant::I_ROOTNOTE));
		y+=sy+sy;
		addParam(createParamCentered<CoffeeKnob8mm>(mm2px(Vec(mx, y)), module, Quant::P_SCALE));
		addInput(createInputCentered<CoffeeInputPort>(mm2px(Vec(mx, y+sy)), module, Quant::I_SCALE));
		y+=sy+sy;
		addParam(createParamCentered<CoffeeKnob8mm>(mm2px(Vec(mx, y)), module, Quant::P_OCTAVE));
		addInput(createInputCentered<CoffeeInputPort>(mm2px(Vec(mx, y+sy)), module, Quant::I_OCTAVE));
		y+=sy+sy;
		addInput(createInputCentered<CoffeeInputPort>(mm2px(Vec(mx, y)), module, Quant::I_VOCT));
		y+=sy;
		addOutput(createOutputCentered<CoffeeOutputPort>(mm2px(Vec(mx, y)), module, Quant::O_VOCT));
	}
};


Model* modelQuant = createModel<Quant, QuantWidget>("Quant");