#include "plugin.hpp"
#include "components.hpp"

struct Quant : Module {
	enum ParamId {
		P_octaveOffset_OFFSET,
		P_semiOffset_OFFSET,
		ENUMS(P_NOTE_BUTTON,12),
		PARAMS_LEN
	};
	enum InputId {
		I_octaveOffset,
		I_semiOffset,
		I_VOCT,
		INPUTS_LEN
	};
	enum OutputId {
		O_VOCT,
		OUTPUTS_LEN
	};
	enum LightId {
		ENUMS(L_NOTE,12*2),
		LIGHTS_LEN
	};

	float _octaveOffset=-1;
	float _lastOctaveOffset=-1;
	float _semiOffset=-1;
	float _lastSemiOffset=-1;
	
	dsp::ClockDivider _lowPriorityClock;

	Quant() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		for(int i=0;i<12;i++) {
			configParam(P_NOTE_BUTTON+i, 0.0, 1.0, 1.0, string::f("Note %d", i+1));
		}

		configParam(P_octaveOffset_OFFSET, -5.f, 5.f, 0.f, "Octave Offset");
		paramQuantities[P_octaveOffset_OFFSET]->snapEnabled = true;
		configParam(P_semiOffset_OFFSET, 0.f, 12.f, 0.f, "Semi Offset", " semitones");
		paramQuantities[P_semiOffset_OFFSET]->snapEnabled = true;
		configInput(I_octaveOffset, "Octave Offset");
		configInput(I_semiOffset, "Semi Offset");
		configInput(I_VOCT, "V/OCT In");
		configOutput(O_VOCT, "V/OCT Out");
		_lowPriorityClock.setDivision(32);
	}

	void process(const ProcessArgs& args) override {
		for (int i = 0; i < 12; i++)
		{
			//set the latch button lights
			//if button to high, set light 
			if(params[P_NOTE_BUTTON+i].getValue()>0.5) {
				lights[L_NOTE+i*2+0].setBrightness(1);
				lights[L_NOTE+i*2+1].setBrightness(0);
			}
			else {
				lights[L_NOTE+i*2+0].setBrightness(0);
				lights[L_NOTE+i*2+1].setBrightness(0);			
			}
		}

		if(inputs[I_octaveOffset].isConnected()) {
			_octaveOffset = clamp((int)inputs[I_octaveOffset].getVoltage(),-5,5);
		} else {
			_octaveOffset = params[P_octaveOffset_OFFSET].getValue();
		}

		//update control if changed
		if(_lastOctaveOffset != _octaveOffset){
			_lastOctaveOffset = _octaveOffset;
			params[P_octaveOffset_OFFSET].setValue(_octaveOffset);
		}

		if(inputs[I_semiOffset].isConnected()) {
			_semiOffset = clamp((int)inputs[I_semiOffset].getVoltage(),-12,12);
		} else {
			_semiOffset = params[P_semiOffset_OFFSET].getValue();
		}

		//update control if changed
		if(_lastSemiOffset != _semiOffset){
			_lastSemiOffset = _semiOffset;
			params[P_semiOffset_OFFSET].setValue(_semiOffset);
		}


		if(inputs[I_VOCT].isConnected()) {
			float voltsIn=inputs[I_VOCT].getVoltage();

			//add the semi offset
			voltsIn+=(_semiOffset * (1.0f/12.0f));

			float bestNote = 10.0;
			float minDiff = 10.0;
			float vNote = 0;
			float vDiff = 0;
			int vOctave = int(floorf(voltsIn));
			float vPitch = voltsIn - vOctave;
			int bestbutton=-1;

			//cycle through the available notes and find the closest one
			for (int i=0; i < 12; i++) {
				if(params[P_NOTE_BUTTON+i].getValue()>0.5){
					vNote = i  / 12.0f;
					vDiff = fabs(vPitch - vNote);
					if(vDiff < minDiff){
						bestNote = vNote;
						minDiff = vDiff;
						bestbutton = i;
					}
				}
			}
			float voltsOut = bestNote + vOctave + _octaveOffset;

			lights[L_NOTE + bestbutton * 2 + 0].setBrightness(0);
			lights[L_NOTE + bestbutton * 2 + 1].setBrightness(0.9f);

			outputs[O_VOCT].setVoltage(voltsOut);
		}
	}
};

template <typename TBase = GrayModuleLightWidget>
struct TOrangeWhiteLight : TBase {
	TOrangeWhiteLight() {
		this->addBaseColor(SCHEME_ORANGE);
		this->addBaseColor(SCHEME_WHITE);
	}
};
using OrangeWhiteLight = TOrangeWhiteLight<>;


struct QuantWidget : ModuleWidget {
	QuantWidget(Quant* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Quant.svg")));

		float yOffset = 15.0;
		float sy=10;
		float width = 10.16;
		float mx =  width/2;
		float y = yOffset;

		addParam(createParamCentered<CoffeeKnob8mm>(mm2px(Vec(mx, y)), module, Quant::P_octaveOffset_OFFSET));
		y+=sy;
		addInput(createInputCentered<CoffeeInputPort>(mm2px(Vec(mx, y)), module, Quant::I_octaveOffset));
		y+=sy*1.5;

		addParam(createParamCentered<CoffeeKnob8mm>(mm2px(Vec(mx, y)), module, Quant::P_semiOffset_OFFSET));
		y+=sy;
		addInput(createInputCentered<CoffeeInputPort>(mm2px(Vec(mx, y)), module, Quant::I_semiOffset));
		y+=15;

		float x=0;
		float sx=2;
		for(int i=11; i>=0; i--) {
			if(i==10 || i==8 || i==6 || i==3 || i==1) {
				x=mx-sx;
			} else {
				x=mx+sx;
			}
			addParam(createParamCentered<Coffee3mmButtonLatch>(mm2px(Vec(x, y)), module, Quant::P_NOTE_BUTTON+i));
			addChild(createLightCentered<MediumSimpleLight<OrangeWhiteLight>>(mm2px(Vec(x, y)), module, Quant::L_NOTE+i*2));

			y+=i==5 ? 4 : 2;  //skip the 5th note's sharp
		}

		y+=8;
		addInput(createInputCentered<CoffeeInputPort>(mm2px(Vec(mx, y)), module, Quant::I_VOCT));
		y+=sy;
		addOutput(createOutputCentered<CoffeeOutputPort>(mm2px(Vec(mx, y)), module, Quant::O_VOCT));

	}
};


Model* modelQuant = createModel<Quant, QuantWidget>("Quant");