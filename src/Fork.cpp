#include "plugin.hpp"

#include "components.hpp"

#define NUM_GROUPS 2

struct Fork: Module {
    enum ParamId {
        ENUMS(P_TRIG_BUTTON, NUM_GROUPS),
            ENUMS(P_PROB, NUM_GROUPS),
            PARAMS_LEN
    };
    enum InputId {
        ENUMS(I_TRIG, NUM_GROUPS),
            ENUMS(I_TOP, NUM_GROUPS),
            ENUMS(I_BOTTOM, NUM_GROUPS),
            ENUMS(I_PROB, NUM_GROUPS),
            INPUTS_LEN
    };
    enum OutputId {
        ENUMS(O_CV, NUM_GROUPS),
            OUTPUTS_LEN
    };
    enum LightId {
        ENUMS(L_HEADS_A, NUM_GROUPS),
            ENUMS(L_HEADS_B, NUM_GROUPS),
            LIGHTS_LEN
    };

    dsp::SchmittTrigger clockTrigger[NUM_GROUPS];
    dsp::BooleanTrigger clockButtonTrigger[NUM_GROUPS];
    dsp::PulseGenerator outPulse[NUM_GROUPS];
    bool ready[NUM_GROUPS] = {
        true,
        true
    };

    Fork() {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
        for (int i = 0; i < NUM_GROUPS; i++) {
            configButton(P_TRIG_BUTTON + i, "Manual Trigger");
            configParam(P_PROB + i, -1.f, 1.f, 1.f, "Chance");
            configInput(I_TRIG + i, string::f("Trig %d", i + 1));
            configInput(I_PROB + i, string::f("Prob %d", i + 1));
            configInput(I_TOP + i, string::f("Input A %d", i + 1));
            configInput(I_BOTTOM + i, string::f("Input B %d", i + 1));
            configOutput(O_CV + i, string::f("Output %d", i + 1));
        }
    }

    void process(const ProcessArgs & args) override {
        for (int i = 0; i < NUM_GROUPS; i++) {
            float i_clock = false;
            if (inputs[I_TRIG + i].isConnected()) {
                i_clock = clockTrigger[i].process(inputs[I_TRIG + i].getVoltage());
            }
            float i_buttom = clockButtonTrigger[i].process(params[P_TRIG_BUTTON + i].getValue());
            if (ready[i] && (i_clock || i_buttom)) {
                float r = (random::uniform() * 2) - 1;
                float p = params[P_PROB].getValue();
                if (inputs[I_PROB + i].isConnected()) {
                    p = inputs[I_PROB + i].getVoltage();
                }
                if (r > p) {
                    lights[L_HEADS_A + i].setBrightness(1);
                    lights[L_HEADS_B + i].setBrightness(0);
                    outputs[O_CV + i].setVoltage(inputs[I_TOP + i].getVoltage());
                } else {
                    lights[L_HEADS_A + i].setBrightness(0);
                    lights[L_HEADS_B + i].setBrightness(1);
                    outputs[O_CV + i].setVoltage(inputs[I_BOTTOM + i].getVoltage());
                }
            }
            ready[i] = (inputs[I_TRIG + i].getVoltage()) ? false : true;
        }
    }
};

struct ForkWidget: ModuleWidget {
    ForkWidget(Fork * module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/Fork.svg")));

        float width = 10.16;
        float mx = width / 2;
        float gOffset = 57;
        float yOffset = 12;
        float s = 10;
        float x = mx;
        float y;
        for (int i = 0; i < NUM_GROUPS; i++) {
            y = yOffset + (gOffset * i);
            addInput(createInputCentered < CoffeeInputPortButton > (mm2px(Vec(x, y)), module, Fork::I_TRIG + i));
            addParam(createParamCentered < CoffeeTinyButton > (mm2px(Vec(x + 3.5, y - 3.5)), module, Fork::P_TRIG_BUTTON + i));

            addInput(createInputCentered < CoffeeInputPort > (mm2px(Vec(x, y + s)), module, Fork::I_PROB + i));
            addParam(createParamCentered < CoffeeKnob6mm > (mm2px(Vec(x, y + s + s - 2)), module, Fork::P_PROB + i));

            y = yOffset + (gOffset * i) + s - 4;
            addInput(createInputCentered < CoffeeInputPortIndicator > (mm2px(Vec(x, y + (s * 2))), module, Fork::I_TOP + i));
            addChild(createLightCentered < CoffeeTinyLight < OrangeLight >> (mm2px(Vec(x + 3.5, y + (s * 2) + 3.5)), module, Fork::L_HEADS_A + i));

            addInput(createInputCentered < CoffeeInputPortIndicator > (mm2px(Vec(x, y + (s * 3))), module, Fork::I_BOTTOM + i));
            addChild(createLightCentered < CoffeeTinyLight < OrangeLight >> (mm2px(Vec(x + 3.5, y + (s * 3) + 3.5)), module, Fork::L_HEADS_B + i));

            addOutput(createOutputCentered < CoffeeOutputPort > (mm2px(Vec(x, y + (s * 4))), module, Fork::O_CV + i));
        }
    }
};

Model * modelFork = createModel < Fork, ForkWidget > ("Fork");