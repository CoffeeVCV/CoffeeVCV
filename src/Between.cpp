#include "plugin.hpp"

#include "components.hpp"

struct Between : Module
{
    enum ParamId
    {
        P_TRIG,
        P_SCALE_A,
        P_SCALE_B,
        P_OFFSET_A,
        P_OFFSET_B,
        PARAMS_LEN
    };
    enum InputId
    {
        I_TRIG,
        I_CV_A,
        I_CV_B,
        INPUTS_LEN
    };
    enum OutputId
    {
        O_CV,
        OUTPUTS_LEN
    };
    enum LightId
    {
        LIGHTS_LEN
    };

    float lastMaxCV = 0.f;
    float lastMinCV = 10.f;
    float hold = 0.f;

    dsp::SchmittTrigger _clockTrigger;
    dsp::BooleanTrigger _buttonTrigger;

    Between()
    {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
        configButton(P_TRIG, "Trigger");
        configParam(P_SCALE_A, -5.f, 5.f, 1.f, "Scale A");
        configParam(P_SCALE_B, -5.f, 5.f, 1.f, "Scale B");
        configParam(P_OFFSET_A, -5.f, 5.f, 0.f, "Offset A");
        configParam(P_OFFSET_B, -5.f, 5.f, 0.f, "Offset B");
        configInput(I_TRIG, "Trigger");
        configInput(I_CV_A, "CV1");
        configInput(I_CV_B, "CV2");
        configOutput(O_CV, "Out");
    }

    void process(const ProcessArgs &args) override
    {
        // no output, no point
        if (!outputs[O_CV].isConnected())
        {
            return;
        }

        float b_trigger = params[P_TRIG].getValue();
        float i_trigger = inputs[I_TRIG].getVoltage();
        if ((_clockTrigger.process(i_trigger) || _buttonTrigger.process(b_trigger)))
        {
            float CV1 = 0;
            float CV2 = 0;

            if (inputs[I_CV_A].isConnected())
                CV1 += inputs[I_CV_A].getVoltage();
            CV1 += params[P_OFFSET_A].getValue();
            CV1 *= params[P_SCALE_A].getValue();

            if (inputs[I_CV_B].isConnected())
                CV2 += inputs[I_CV_B].getVoltage();
            CV2 += params[P_OFFSET_B].getValue();
            CV2 *= params[P_SCALE_B].getValue();

            if (CV1 < CV2)
            {
                hold = ((CV1 - CV2) * random::uniform()) + CV2;
            }
            else
            {
                hold = ((CV2 - CV1) * random::uniform()) + CV1;
            }
        }

        outputs[O_CV].setVoltage(hold);
    }
};

struct BetweenWidget : ModuleWidget
{
    BetweenWidget(Between *module)
    {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/Between.svg")));

        float x = 7.62;
        float y = 15;
        addInput(createInputCentered<CoffeeInputPortButton>(mm2px(Vec(x, y)), module, Between::I_TRIG));
        addParam(createParamCentered<CoffeeTinyButton>(mm2px(Vec(x + 3.5, y - 3.5)), module, Between::P_TRIG));

        addInput(createInputCentered<CoffeeInputPort>(mm2px(Vec(x, 35)), module, Between::I_CV_A));
        addParam(createParamCentered<CoffeeKnob6mm>(mm2px(Vec(x, 45)), module, Between::P_OFFSET_A));
        addParam(createParamCentered<CoffeeKnob6mm>(mm2px(Vec(x, 55)), module, Between::P_SCALE_A));

        addInput(createInputCentered<CoffeeInputPort>(mm2px(Vec(x, 75)), module, Between::I_CV_B));
        addParam(createParamCentered<CoffeeKnob6mm>(mm2px(Vec(x, 85)), module, Between::P_OFFSET_B));
        addParam(createParamCentered<CoffeeKnob6mm>(mm2px(Vec(x, 95)), module, Between::P_SCALE_B));

        addOutput(createOutputCentered<CoffeeOutputPort>(mm2px(Vec(x, 112)), module, Between::O_CV));
    }
};

Model *modelBetween = createModel<Between, BetweenWidget>("Between");