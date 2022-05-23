#include "plugin.hpp"

#include "components.hpp"

#define NUM_ROWS 8

struct Tumble: Module {
    enum ParamId {
        P_TRIG_BUTTON,
        P_RESET_BUTTON,
        P_START_BUTTON,
		P_MODE_BUTTON,
        ENUMS(P_COUNTER, NUM_ROWS),
        PARAMS_LEN
    };
    enum InputId {
        I_TRIG,
        I_RESET,
        I_START,
        INPUTS_LEN
    };
    enum OutputId {
        ENUMS(O_TRIG, NUM_ROWS),
            ENUMS(O_GATE, NUM_ROWS),
            O_ORTRIG,
            O_ORGATE,
            O_EOC,
            OUTPUTS_LEN
    };
    enum LightId {
        ENUMS(L_PACTIVE, NUM_ROWS),
            ENUMS(L_PTRIG, NUM_ROWS),
            ENUMS(L_PROGRESS, NUM_ROWS),
            L_START,
			L_ONCEMODE,
			L_LOOPMODE,
            LIGHTS_LEN
    };

	enum Mode {
		ONCE,
		LOOP
	};

    int counts[NUM_ROWS];
    int initialCount[NUM_ROWS];
    int currentCounter = 0;
    int eocRow;
    bool runState = false;
    // bool runToggleReady = true;
	bool modeToggleReady = true;
	int Mode;

    dsp::SchmittTrigger clockTrigger;
    dsp::SchmittTrigger startTrigger;
    dsp::SchmittTrigger resetTrigger;
    dsp::BooleanTrigger clockButtonTrigger;
    dsp::BooleanTrigger startButtonTrigger;
    dsp::BooleanTrigger modeButtonTrigger;
    dsp::BooleanTrigger resetButtonTrigger;
    dsp::PulseGenerator rowPulse[NUM_ROWS];
    dsp::PulseGenerator eocPulse;
    dsp::PulseGenerator globalPulse;

    Tumble() {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
        configParam(P_TRIG_BUTTON, 0.f, 1.f, 0.f, "Manual Trig");
        configInput(I_TRIG, "Clock Trig");
        configParam(P_RESET_BUTTON, 0.f, 1.f, 0.f, "Manual Reset");
        configInput(I_RESET, "Reset Trig");
        configParam(P_START_BUTTON, 0.f, 1.f, 0.f, "Manual Start");
        configInput(I_START, "Stat Trig");
        configButton(P_MODE_BUTTON,"Once/Loop Mode");
        for (int i = 0; i < NUM_ROWS; i++) {
            configParam(P_COUNTER + i, 0.f, 16.f, 0.f, string::f("Counter %d", i + 1));
            paramQuantities[P_COUNTER + i] -> snapEnabled = true;
            configOutput(O_TRIG + i, string::f("Trigger %d", i + 1));
            configOutput(O_GATE + i, string::f("Gate %d", i + 1));
        }
        configOutput(O_EOC, "End of Cycle");
        configOutput(O_ORTRIG, "Global Trig");
        configOutput(O_ORGATE, "Global Gate");
		Mode=LOOP;
		ToggleMode();
        reset();
    }

    void reset() {
        for (int i = 0; i < NUM_ROWS; i++) {
            initialCount[i] = params[P_COUNTER + i].getValue();
            outputs[O_TRIG + i].setVoltage(0.f);
            outputs[O_GATE + i].setVoltage(0.f);
            counts[i] = initialCount[i];
        }
		outputs[O_ORGATE].setVoltage(0.f);
		outputs[O_ORTRIG].setVoltage(0.f);
        currentCounter = 0;
    }

	void ToggleMode() {
		if(Mode==ONCE) {
			Mode=LOOP;
			lights[L_ONCEMODE].setBrightness(0);
			lights[L_LOOPMODE].setBrightness(1);
		} else {
			Mode=ONCE;
			lights[L_ONCEMODE].setBrightness(1);
			lights[L_LOOPMODE].setBrightness(0);
		}		
	}

    void process(const ProcessArgs & args) override {
        bool clock = clockTrigger.isHigh();
        float pulse;

        if (startTrigger.process(inputs[I_START].getVoltage() || startButtonTrigger.process(params[P_START_BUTTON].getValue()))) {
            runState = (runState) ? false : true;
            lights[L_START].setBrightness((runState) ? 1 : 0);
        }

        if (resetTrigger.process(inputs[I_RESET].getVoltage() || resetButtonTrigger.process(params[P_RESET_BUTTON].getValue()))) {
            reset();
        }

		// Mode 
		if ( modeButtonTrigger.process(params[P_MODE_BUTTON].getValue())) {
			ToggleMode();
		}

        pulse = eocPulse.process(args.sampleTime);
        outputs[O_EOC].setVoltage(pulse ? 10.f : 0.f);

        pulse = globalPulse.process(args.sampleTime);
        outputs[O_ORTRIG].setVoltage(pulse ? 10.f : 0.f);
        outputs[O_ORGATE].setVoltage(clock ? 10.f : 0.f);

        eocRow = -1;

        // process each row
        for (int i = 0; i < NUM_ROWS; i++) {
            // set the little indicator if the counter is active
            if (params[P_COUNTER + i].getValue() > 0) {
                lights[L_PACTIVE + i].setBrightness(1);
            } else {
                lights[L_PACTIVE + i].setBrightness(0);
            }

            // if the param changed, reset the values
            if (params[P_COUNTER + i].getValue() != initialCount[i]) {
                initialCount[i] = params[P_COUNTER + i].getValue();
                counts[i] = initialCount[i];
            }

            // find eoc, highest numbered counter with non-zero start
            if (initialCount[i] > 0) eocRow = i;

            pulse = rowPulse[i].process(args.sampleTime);
            outputs[O_TRIG + i].setVoltage(pulse ? 10.f : 0);
            outputs[O_GATE + i].setVoltage((clock && i == currentCounter) ? 10.f : 0.f);

            lights[L_PTRIG + i].setBrightnessSmooth(pulse ? 1 : 0, args.sampleTime);
            float v = (initialCount[i] > 0 ? float(counts[i]) / float(initialCount[i]) : 0);
            lights[L_PROGRESS + i].setBrightnessSmooth(v, args.sampleTime);
        }

        if (runState && clockTrigger.process(inputs[I_TRIG].getVoltage() || clockButtonTrigger.process(params[P_TRIG_BUTTON].getValue()))) {
            //process each counter
			for (int i = 0; i < NUM_ROWS; i++) {
				//if this counter is not finished
                if (counts[i] > 0) {
					//pulse
                    rowPulse[i].trigger(1e-3f);
                    globalPulse.trigger(1e-3f);
                    counts[i]--;
					//if that was the last pulse, do eoc
                    if (i == eocRow && counts[i] == 0) {
                        eocPulse.trigger(1e-3f);
                        reset();
						if(Mode==ONCE) {
							runState = false;;
            				lights[L_START].setBrightness(0);
						}
                    }
                    currentCounter = i;
                    break;
                }
            }
        }
    }
};

struct TumbleWidget: ModuleWidget {
    TumbleWidget(Tumble * module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/Tumble.svg")));

        float x, y;
        float s = 10;
        float lm = 15.12 / 2;
        float xoffset = lm;
        float yoffset = 27;

        x = xoffset;
        y = 13;
        addInput(createInputCentered < CoffeeInputPortButton > (mm2px(Vec(x, y)), module, Tumble::I_TRIG));
        addParam(createParamCentered < CoffeeTinyButton > (mm2px(Vec(x + 3.5, y - 3.5)), module, Tumble::P_TRIG_BUTTON));
        addInput(createInputCentered < CoffeeInputPortButton > (mm2px(Vec(x + s, y)), module, Tumble::I_START));
        addParam(createParamCentered < CoffeeTinyButton > (mm2px(Vec(x + s + 3.5, y - 3.5)), module, Tumble::P_START_BUTTON));
        addChild(createLightCentered < MediumLight < GreenLight >> (mm2px(Vec(x + s, y + 7)), module, Tumble::L_START));
        addInput(createInputCentered < CoffeeInputPortButton > (mm2px(Vec(x + (s*3), y)), module, Tumble::I_RESET));
        addParam(createParamCentered < CoffeeTinyButton > (mm2px(Vec(x + (s*3) + 3.5, y - 3.5)), module, Tumble::P_RESET_BUTTON));

		//mode
		x=x + (s*2);
		addParam(createParamCentered<CoffeeButtonVertIndicator>(mm2px(Vec(x, y)), module, Tumble::P_MODE_BUTTON));
		addChild(createLightCentered<TinyLight<OrangeLight>>(mm2px(Vec(x, y)), module, Tumble::L_ONCEMODE));
		addChild(createLightCentered<TinyLight<OrangeLight>>(mm2px(Vec(x, y+2)), module, Tumble::L_LOOPMODE));



        x = xoffset;
        for (int i = 0; i < NUM_ROWS; i++) {
            y = yoffset + (s * i);
            addParam(createParamCentered < RoundBlackKnob > (mm2px(Vec(x, y)), module, Tumble::P_COUNTER + i));
            addChild(createLightCentered < TinyLight < YellowLight >> (mm2px(Vec(x + 6, y)), module, Tumble::L_PACTIVE + i));
            addChild(createLightCentered < MediumLight < YellowLight >> (mm2px(Vec(x + s, y - 2)), module, Tumble::L_PROGRESS + i));
            addChild(createLightCentered < MediumLight < GreenLight >> (mm2px(Vec(x + s, y + 2)), module, Tumble::L_PTRIG + i));
            addOutput(createOutputCentered < CoffeeOutputPort > (mm2px(Vec(x + (s * 2), y)), module, Tumble::O_TRIG + i));
            addOutput(createOutputCentered < CoffeeOutputPort > (mm2px(Vec(x + (s * 3), y)), module, Tumble::O_GATE + i));
        }

        x = xoffset + s;
        y = yoffset + (s * (NUM_ROWS)) + (s / 2);
        addOutput(createOutputCentered < CoffeeOutputPort > (mm2px(Vec(x, y)), module, Tumble::O_EOC));
        addOutput(createOutputCentered < CoffeeOutputPort > (mm2px(Vec(x + s, y)), module, Tumble::O_ORTRIG));
        addOutput(createOutputCentered < CoffeeOutputPort > (mm2px(Vec(x + s + s, y)), module, Tumble::O_ORGATE));

    }
};

Model * modelTumble = createModel < Tumble, TumbleWidget > ("Tumble");