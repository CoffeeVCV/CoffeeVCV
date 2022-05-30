#include "plugin.hpp"

#include "components.hpp"

#define NUM_GROUPS 2

struct Set: Module {
    enum ParamId {
        ENUMS(P_SCALE, NUM_GROUPS),
            ENUMS(P_OFFSET, NUM_GROUPS),
            PARAMS_LEN
    };
    enum InputId {
        ENUMS(I_SCALE, NUM_GROUPS),
            ENUMS(I_OFFSET, NUM_GROUPS),
            ENUMS(I_CV, NUM_GROUPS),
            INPUTS_LEN
    };
    enum OutputId {
        ENUMS(O_CV, NUM_GROUPS),
            O_DEBUG_OUTPUT,
            OUTPUTS_LEN
    };
    enum LightId {
        LIGHTS_LEN
    };

    Set() {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
        for (int i = 0; i < NUM_GROUPS; i++) {
            configParam(P_SCALE + i, -5.f, 5.f, 1.f, string::f("Scale %d", i + 1));
            configParam(P_OFFSET + i, -5.f, 5.f, 0.f, string::f("Offset %d", i + 1));
            configInput(I_SCALE + i, string::f("Scale %d CV", i + 1));
            configInput(I_OFFSET + i, string::f("Offset %d CV", i + 1));
            configInput(I_CV + i, string::f("Input %d", i + 1));
            configOutput(O_CV + i, string::f("Output %d", i + 1));
        }
        configOutput(O_DEBUG_OUTPUT, "");
    }

    void process(const ProcessArgs & args) override {
        for (int i = 0; i < NUM_GROUPS; i++) {
            //get the input
            float outValue = inputs[I_CV + i].getVoltage();

            //add the offset
            if (inputs[I_OFFSET + i].isConnected()) {
                outValue += inputs[I_OFFSET + i].getVoltage();
            } else {
                outValue += params[P_OFFSET + i].getValue();
            }

            //scale it
            if (inputs[I_SCALE + i].isConnected()) {
                outValue *= inputs[I_SCALE + i].getVoltage();
            } else {
                outValue *= params[P_SCALE + i].getValue();
            }

            //send to out
            outputs[O_CV + i].setVoltage(outValue);
        }
    }
};

struct SetWidget: ModuleWidget {
    SetWidget(Set * module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/Set.svg")));
        float width = 10.16;
        float xOffset = width / 2;
        float yOffset = 12;
        float gs = 57;
        float s = 10;
        float x = xOffset;
        for (int i = 0; i < NUM_GROUPS; i++) {
            float y = yOffset + (gs * i);
            addInput(createInputCentered < CoffeeInputPort > (mm2px(Vec(x, y)), module, Set::I_CV + i));
            addInput(createInputCentered < CoffeeInputPort > (mm2px(Vec(x, y + s)), module, Set::I_OFFSET + i));
            addParam(createParamCentered < CoffeeKnob6mm > (mm2px(Vec(x, y + s + s - 2)), module, Set::P_OFFSET + i));
            y = yOffset + (gs * i) + s + s - 2 + s;
            addInput(createInputCentered < CoffeeInputPort > (mm2px(Vec(x, y)), module, Set::I_SCALE + i));
            addParam(createParamCentered < CoffeeKnob6mm > (mm2px(Vec(x, y + s - 2)), module, Set::P_SCALE + i));
            addOutput(createOutputCentered < CoffeeOutputPort > (mm2px(Vec(x, y + s + s - 2)), module, Set::O_CV + i));
        }
    }
};

Model * modelSet = createModel < Set, SetWidget > ("Set");