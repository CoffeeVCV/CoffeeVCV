#include "plugin.hpp"
#include "components.hpp"
#define NUM_ROWS 8
#define NUM_GROUPS 4

// [ ] Todo - add polyphony to input
// [ ] Todo - add polyphony to output


struct These : Module {
	enum ParamId {
		ENUMS(P_BUTTON,NUM_ROWS * NUM_GROUPS),
		PARAMS_LEN
	};
	enum InputId {
		ENUMS(I_ROW,NUM_ROWS),
		INPUTS_LEN
	};
	enum OutputId {
		ENUMS(O_GROUP,NUM_GROUPS),
		OUTPUTS_LEN
	};
	enum LightId {
		ENUMS(L_LATCH,NUM_ROWS * NUM_GROUPS),
		ENUMS(L_OUT,NUM_GROUPS),
		LIGHTS_LEN
	};

	// enum Groups {
	// 	A,
	// 	B,
	// 	C,
	// 	D
	// };

	dsp::SchmittTrigger _rowTrigger[NUM_ROWS];
	dsp::PulseGenerator _outPulse[NUM_GROUPS];
	dsp::ClockDivider _lowPriority;

	bool _in[NUM_ROWS];
	bool _ready[NUM_ROWS];
	int _matrix[NUM_ROWS][NUM_GROUPS];

	These() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		for(int i=0; i<NUM_ROWS; i++){		
			for(int j=0; j<NUM_GROUPS; j++){
				configInput(I_ROW+i, "Row In");
				int n=i+(j*NUM_ROWS);
				DEBUG("n: %i", n);
				configButton(P_BUTTON + n,string::f("Row %d, Group %d, #%d", i, j, n));
			}	
		}
		for(int j=0;j<NUM_GROUPS;j++){
			configOutput(O_GROUP + j, string::f("Group %d out",j));
		}
		_lowPriority.setDivision(16);

		for(int i=0; i<NUM_ROWS; i++){
			_ready[i]=true;	
			for(int j=0; j<NUM_GROUPS; j++){
				_matrix[i][j]=0;
			}
		}
	}

	void process(const ProcessArgs& args) override {
		if(_lowPriority.process()){
			//set lights for latch button values
			for(int i=0; i<NUM_ROWS; i++){
				for(int j=0; j<NUM_GROUPS;j++){
					int n=i+(j*NUM_ROWS);
					lights[L_LATCH + n].setBrightness(params[P_BUTTON + n].getValue());
				}
			}
		}

		// check inputs, if button is latched, send to output
		for(int i=0; i<NUM_ROWS; i++){
			float v=clamp(inputs[I_ROW + i].getVoltage(),0.f,2.f);
			_in[i]=_rowTrigger[i].process(v);
			if(_in[i] && _ready[i]){		
				for(int j=0; j<NUM_GROUPS;j++){	
					int n=i+(j*NUM_ROWS);		
					_matrix[i][j]=params[P_BUTTON+n].getValue();
				}
				_ready[i]=false;
			}
		}

		// if the input is not triggered, update the matrix
		for(int i=0; i<NUM_ROWS; i++){
			_ready[i]=!_rowTrigger[i].isHigh();
			if(_ready[i]) {
				for(int g=0; g<NUM_GROUPS; g++){
					_matrix[i][g]=0;
				}
			}	
		}

		// check the matrix for output
		//check each row
		for(int j=0; j<NUM_GROUPS; j++){
			int v=0;
			//count the number of active rows, per group
			for(int i=0; i<NUM_ROWS; i++) {
				v+=_matrix[i][j];
			}	
			if(v>0) {
				outputs[O_GROUP + j].setVoltage(10);
				lights[L_OUT + j].setBrightness(1);
			} else {
				outputs[O_GROUP + j].setVoltage(0);
				lights[L_OUT + j].setBrightness(0);
			}
		}
	}
};


struct TheseWidget : ModuleWidget {
	enum {
		A,
		B,
		C,
		D
	};

	TheseWidget(These* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/These.svg")));
		float width=20.32;
		float yoffset=16;
		float s=10;
		float x,y;
		float lx=width/4;
		float rx=lx*3;

		for(int i=0;i<NUM_ROWS;i++){
			y=yoffset+(s*i);
			addInput(createInputCentered<CoffeeInputPort>(mm2px(Vec(lx, y)), module, These::I_ROW + i));

			addParam(createParamCentered<Coffee3mmButtonLatch>(mm2px(Vec(rx-2, y-2)), module, These::P_BUTTON + i + (A * NUM_ROWS) ));
			addParam(createParamCentered<Coffee3mmButtonLatch>(mm2px(Vec(rx+2, y-2)), module, These::P_BUTTON + i + (B * NUM_ROWS) ));
			addParam(createParamCentered<Coffee3mmButtonLatch>(mm2px(Vec(rx-2, y+2)), module, These::P_BUTTON + i + (C * NUM_ROWS) ));
			addParam(createParamCentered<Coffee3mmButtonLatch>(mm2px(Vec(rx+2, y+2)), module, These::P_BUTTON + i + (D * NUM_ROWS) ));

			addChild(createLightCentered<Coffee3mmSimpleLight<OrangeLight>>(mm2px(Vec(rx-2, y-2)), module, These::L_LATCH + i + (A * NUM_ROWS) ));
			addChild(createLightCentered<CoffeeTinyLight<OrangeLight>>(mm2px(Vec(rx+2, y-2)), module, These::L_LATCH + i + (B * NUM_ROWS) ));
			addChild(createLightCentered<CoffeeTinyLight<OrangeLight>>(mm2px(Vec(rx-2, y+2)), module, These::L_LATCH + i + (C * NUM_ROWS) ));
			addChild(createLightCentered<CoffeeTinyLight<OrangeLight>>(mm2px(Vec(rx+2, y+2)), module, These::L_LATCH + i + (D * NUM_ROWS) ));
		}

		y=yoffset+(s*NUM_ROWS)+s/2;
		addOutput(createOutputCentered<CoffeeOutputPortIndicator>(mm2px(Vec(lx,y )), module, These::O_GROUP + A));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(lx+3.5, y+3.5)), module, These::L_OUT + A ));

		addOutput(createOutputCentered<CoffeeOutputPortIndicator>(mm2px(Vec(rx,y )), module, These::O_GROUP + B));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(rx+3.5, y+3.5)), module, These::L_OUT + B ));

		addOutput(createOutputCentered<CoffeeOutputPortIndicator>(mm2px(Vec(lx,y+s )), module, These::O_GROUP + C));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(lx+3.5, y+s+3.5)), module, These::L_OUT + C ));

		addOutput(createOutputCentered<CoffeeOutputPortIndicator>(mm2px(Vec(rx,y+s )), module, These::O_GROUP + D));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(rx+3.5, y+s+3.5)), module, These::L_OUT + D ));
	}
};


Model* modelThese = createModel<These, TheseWidget>("These");