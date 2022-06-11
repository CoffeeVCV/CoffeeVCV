#include "plugin.hpp"
#include "components.hpp"
#define NUM_CHANNELS 16

struct Some3 : Module
{
	enum ParamId
	{
		P_TRIGBUTTON,
		P_SELECTIONSTART,
		P_SELECTIONEND,
		P_PROB,
		PARAMS_LEN
	};
	enum InputId
	{
		I_TRIG,
		I_PROB,
		I_SELECTIONSTART,
		I_SELECTIONEND,
		I_SEED,
		I_CV,
		INPUTS_LEN
	};
	enum OutputId
	{
		O_CV,
		OUTPUTS_LEN
	};
	enum LightId
	{
		ENUMS(L_SELECTED, NUM_CHANNELS),
		ENUMS(L_ACTIVE, NUM_CHANNELS),
		L_SELECTIONSTART_ERR,
		L_SELECTIONEND_ERR,
		L_PROB_ERR,
		LIGHTS_LEN
	};

	enum SelectionMode
	{
		ENDPOS,
		LENGTH
	};

	int _selectionStart = 0;
	int _selectionEnd = 0;
	int _selectionLength = 0;
	int _outputs[NUM_CHANNELS];
	int _[NUM_CHANNELS];
	bool _selectionStartErr = false;
	bool _selectionEndErr = false;
	dsp::SchmittTrigger _trigTrigger;
	dsp::BooleanTrigger _trigButtonTrigger;
	dsp::ClockDivider _lowPriority;


	Some3()
	{
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configButton(P_TRIGBUTTON, "Manual Trigger");
		configParam(P_SELECTIONSTART, 0.f, NUM_CHANNELS, 0, "Selection Start");
		paramQuantities[P_SELECTIONSTART]->snapEnabled = true;
		configParam(P_SELECTIONEND, 0.f, NUM_CHANNELS, NUM_CHANNELS, "Selection End");
		paramQuantities[P_SELECTIONEND]->snapEnabled = true;
		configParam(P_PROB, 0.f, 1.f, 0.5f, "Probabilty");
		configInput(I_TRIG, "Trigger CV");
		configInput(I_PROB, "Probability CV");
		configInput(I_SELECTIONSTART, "Selection Start CV");
		configInput(I_SELECTIONEND, "Selection End CV");
		configInput(I_CV, "Input CV");
		configInput(I_SEED, "Seed CV");
		configOutput(O_CV, "CV Out 1");
		_lowPriority.setDivision(16);
	}

	void process(const ProcessArgs &args) override
	{
		//check slection input out of bounds errors
		_selectionStartErr=(inputs[I_SELECTIONSTART].getVoltage() < 0.f || inputs[I_SELECTIONSTART].getVoltage() > NUM_CHANNELS);
		_selectionEndErr=(inputs[I_SELECTIONEND].getVoltage() < 0.f || inputs[I_SELECTIONEND].getVoltage() > NUM_CHANNELS);

		if(_lowPriority.process()){
			// update lighsts to show selection
			for(int i=0; i<NUM_CHANNELS; i++){
				if(i>=params[P_SELECTIONSTART].getValue() && i<params[P_SELECTIONEND].getValue()){
					lights[L_SELECTED + i].setBrightness(1.0f);
				}
				else{
					lights[L_SELECTED + i].setBrightness(0.0f);
				}
			}

			// update prob light is out of bounds
			if(inputs[I_PROB].getVoltage() < 0.f || inputs[I_PROB].getVoltage() > 1.f){
				lights[L_PROB_ERR].setBrightness(1.0f);
			}
			else{
				lights[L_PROB_ERR].setBrightness(0.0f);
			}

			// update selection error lights
			lights[L_SELECTIONSTART_ERR].setBrightness(_selectionStartErr);
			lights[L_SELECTIONEND_ERR].setBrightness(_selectionEndErr);
		}

		//check tigger and trigger button
		bool trigButtonPressed = _trigButtonTrigger.process(params[P_TRIGBUTTON].getValue());
		bool trigTriggered = _trigTrigger.process(inputs[I_TRIG].getVoltage());
		if(trigButtonPressed | trigTriggered){
			//use select start input if connected 
			if(inputs[I_SELECTIONSTART].isConnected()){
				_selectionStart = clamp(1+(int)inputs[I_SELECTIONSTART].getVoltage(), 0, NUM_CHANNELS);
			} else {
				 _selectionStart = 1+int(params[P_SELECTIONSTART].getValue());
			}

			//use select end input if connected
			if(inputs[I_SELECTIONEND].isConnected()){
				_selectionEnd = clamp(1+(int)inputs[I_SELECTIONEND].getVoltage(), 0, NUM_CHANNELS);
			} else {
				_selectionEnd = 1+int(params[P_SELECTIONEND].getValue());
			}

			_selectionLength = _selectionEnd - _selectionStart;
			


			DEBUG("Selection Start: %d", _selectionStart);
			DEBUG("Selection End: %d", _selectionEnd);
			DEBUG("Selection Length: %d", _selectionLength);

			float prob = params[P_PROB].getValue();
			DEBUG("Probability: %f", prob);
			int targetActive = _selectionLength * prob;
			DEBUG("Target Active: %d", targetActive);

			// init the main array
			for(int i=0; i<NUM_CHANNELS;i++){
				_outputs[i] = -1;
			}		

			//temp array to store potential outputs
			int scope[_selectionLength];
			for(int i=0; i<_selectionLength; i++){
				DEBUG("Scope[%d]: %d", i, i+_selectionStart-1);
				scope[i] = i+_selectionStart-1;
			}

			//shuffle array
			for(int i=0; i<_selectionLength; i++){
				int j = rand() % _selectionLength;
				DEBUG("Swap %d with %d", i, j);
				std::swap(scope[i], scope[j]);
			}

			// use only the first quantity from the scope
			for(int i=0; i<targetActive; i++){
				_outputs[scope[i]] = scope[i];
				DEBUG("scope[%d] = %d", i, scope[i]);
			}

			// process the output array
			for(int i=0; i<NUM_CHANNELS;i++){
				if(_outputs[i] == -1){
					lights[L_ACTIVE + i].setBrightness(0.0f);
				}
				else{
					lights[L_ACTIVE + i].setBrightness(1.0f);
				}
			}

			
		}
	}
};

struct Some3Widget : ModuleWidget
{
	Some3Widget(Some3 *module)
	{
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Some3.svg")));

		float width = 20.32;
		float xOffset = 5;
		float yOffset = 15;
		float lx=width/4;
		float rx=lx * 3;
		float sx = 10;
		float sy = 10;
		float x = xOffset;
		float y = yOffset;

		addInput(createInputCentered<CoffeeInputPortButton>(mm2px(Vec(lx, y)), module, Some3::I_TRIG));
		addParam(createParamCentered<Coffee3mmButton>(mm2px(Vec(lx + 3.5, y - 3.5)), module, Some3::P_TRIGBUTTON));

		y += sy;
		addInput(createInputCentered<CoffeeInputPort>(mm2px(Vec(rx, y)), module, Some3::I_SEED));
		addInput(createInputCentered<CoffeeInputPortIndicator>(mm2px(Vec(lx, y)), module, Some3::I_PROB));
		addChild(createLightCentered<SmallLight<RedLight> >(mm2px(Vec(lx+3.5, y + 3.5)), module, Some3::L_PROB_ERR));

		y += sy;
		addParam(createParamCentered<CoffeeKnob6mm>(mm2px(Vec(lx, y)), module, Some3::P_PROB));

		y=yOffset+sy+sy+sy+2.5;
		addInput(createInputCentered<CoffeeInputPortIndicator>(mm2px(Vec(lx, y)), module, Some3::I_SELECTIONSTART));
		addChild(createLightCentered<SmallLight<RedLight> >(mm2px(Vec(lx+3.5, y + 3.5)), module, Some3::L_SELECTIONSTART_ERR));
		y += sy;
		addParam(createParamCentered<CoffeeKnob8mm>(mm2px(Vec(lx, y)), module, Some3::P_SELECTIONSTART));

		y=yOffset+sy*7;
		y+= sy - 2.5;
		addInput(createInputCentered<CoffeeInputPortIndicator>(mm2px(Vec(lx, y)), module, Some3::I_SELECTIONEND));
		addChild(createLightCentered<SmallLight<RedLight> >(mm2px(Vec(lx+3.5, y + 3.5)), module, Some3::L_SELECTIONEND_ERR));
		y += sy;
		addParam(createParamCentered<CoffeeKnob8mm>(mm2px(Vec(lx, y)), module, Some3::P_SELECTIONEND));



		//input and output
		y=yOffset+(sy*10);
		addInput(createInputCentered<CoffeeInputPort>(mm2px(Vec(lx, y)), module, Some3::I_CV));
		y=yOffset+(sy*10);
		addOutput(createOutputCentered<CoffeeOutputPort>(mm2px(Vec(rx, y)), module, Some3::O_CV));

		y=yOffset+sy+sy+sy;
		for (int i = 0; i < NUM_CHANNELS; i++){
			addChild(createLightCentered<MediumLight<OrangeLight> >(mm2px(Vec(rx-2, y + (i * 4))), module, Some3::L_SELECTED + i));
			addChild(createLightCentered<MediumLight<GreenLight> >(mm2px(Vec(rx+ 2, y + (i * 4))), module, Some3::L_ACTIVE + i));
		}
	}
};

Model *modelSome3 = createModel<Some3, Some3Widget>("Some3");