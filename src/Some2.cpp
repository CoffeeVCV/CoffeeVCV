#include "plugin.hpp"

#include "components.hpp"

#define NUM_OUTS 8
struct Some2 : Module
{
    enum ParamId
    {
        P_TRIG,
        P_TRIGBUTTON,
        P_SELECT,
        P_PROB,
        PARAMS_LEN
    };
    enum InputId
    {
        I_TRIG,
        I_CV,
        I_PROB,
        I_SELECT,
        INPUTS_LEN
    };
    enum OutputId
    {
        ENUMS(O_CV, NUM_OUTS),
        OUTPUTS_LEN
    };
    enum LightId
    {
        ENUMS(L_ACTIVE, NUM_OUTS),
        ENUMS(L_SELECT, NUM_OUTS),
        LIGHTS_LEN
    };

    dsp::SchmittTrigger _trigTrigger;
    dsp::BooleanTrigger _trigButtonTrigger;
    dsp::ClockDivider _lowPriority;
    int outs[NUM_OUTS];

    Some2()
    {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
        configButton(P_TRIGBUTTON, "Manual Trigger");
        configParam(P_SELECT, 0.f, NUM_OUTS, NUM_OUTS, "Selection");
        paramQuantities[P_SELECT]->snapEnabled = true;
        configParam(P_PROB, 0.f, 1.f, 0.f, "Probabilty");
        configInput(I_TRIG, "Trigger CV");
        configInput(I_CV, "Input CV");
        configInput(I_PROB, "Probability CV");
        configInput(I_SELECT, "Selection CV");

        for (int i = 0; i < NUM_OUTS; i++)
        {
            configOutput(O_CV + i, string::f("CV Out %d", i + 1));
            configLight(L_ACTIVE + i, string::f("Active %d", i + 1));
            configLight(L_SELECT + i, string::f("Selected %d", i + 1));
        }
        _lowPriority.setDivision(16);
    }

    void process(const ProcessArgs &args) override
    {
        int selected;
        // how many in scope for being selected
        if (inputs[I_SELECT].isConnected())
        {
            selected = clamp(int(inputs[I_SELECT].getVoltage()), 0, NUM_OUTS);
        }
        else
        {
            selected = int(params[P_SELECT].getValue());
        }

        bool trigButtonPressed = _trigButtonTrigger.process(params[P_TRIGBUTTON].getValue());
        bool trigTriggered = _trigTrigger.process(inputs[I_TRIG].getVoltage());
        if (trigButtonPressed || trigTriggered)
        {

            float p;
            if (inputs[I_PROB].isConnected())
            {
                p = clamp(inputs[I_PROB].getVoltage(), 0.f, 1.f);
            }
            else
            {
                p = params[P_PROB].getValue();
            }

            // target number of outputs
            int targetOutputs = (int(selected * p));

            // mask of outputs to be selected
            for (int i = 0; i < NUM_OUTS; i++)
            {
                outs[i] = (i < selected) ? i : -1;
            }

            // shuffle outputs
            for (int i = 0; i < NUM_OUTS; i++)
            {
                int j = (int)(random::uniform() * NUM_OUTS - 1);
                std::swap(outs[i], outs[j]);
            }

            // activate outputs
            int c = 0;
            for (int i = 0; i < NUM_OUTS; i++)
            {
                int n = outs[i];
                if (n != -1)
                {
                    if (c < targetOutputs)
                    {
                        lights[L_ACTIVE + n].setBrightness(1);
                        outputs[O_CV + n].setVoltage(inputs[I_CV].getVoltage());
                        c++;
                    }
                    else
                    {
                        lights[L_ACTIVE + n].setBrightness(0);
                        outputs[O_CV + n].setVoltage(0);
                    }
                }

                if (i >= selected)
                {
                    lights[L_ACTIVE + i].setBrightness(0);
                    outputs[O_CV + i].setVoltage(0);
                }
            }
        }

        if (_lowPriority.process())
        {
            for (int i = 0; i < NUM_OUTS; i++)
            {
                lights[L_SELECT + i].setBrightness((i < selected) ? 1 : 0);
            }
        }
    }
};

struct Some2Widget : ModuleWidget
{
    Some2Widget(Some2 *module)
    {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/Some2.svg")));

        float xOffset = 5.0;
        float yOffset = 12.0;
        float sx = 10;
        float sy = 10;
        float x = xOffset;
        float y = yOffset;
        // input
        addInput(createInputCentered<CoffeeInputPort>(mm2px(Vec(x, y)), module, Some2::I_CV));
        // trigger + button
        addInput(createInputCentered<CoffeeInputPortButton>(mm2px(Vec(x + sx, y)), module, Some2::I_TRIG));
        addParam(createParamCentered<CoffeeTinyButton>(mm2px(Vec(x + sx + 3.5, y - 3.5)), module, Some2::P_TRIGBUTTON));
        y = yOffset + sy;
        addInput(createInputCentered<CoffeeInputPort>(mm2px(Vec(x, y)), module, Some2::I_SELECT));
        addParam(createParamCentered<CoffeeKnob6mm>(mm2px(Vec(x + sx, y)), module, Some2::P_SELECT));

        // probability
        y = yOffset + (sy * 2);
        addInput(createInputCentered<CoffeeInputPort>(mm2px(Vec(x, y)), module, Some2::I_PROB));
        addParam(createParamCentered<CoffeeKnob6mm>(mm2px(Vec(x + sx, y)), module, Some2::P_PROB));

        y = yOffset + sy;
        for (int i = 0; i < NUM_OUTS; i++)
        {
            y = yOffset + sy + sy + sy + (sy * i);
            addOutput(createOutputCentered<CoffeeOutputPort>(mm2px(Vec(x + sx, y)), module, Some2::O_CV + i));
            addChild(createLightCentered<MediumLight<GreenLight> >(mm2px(Vec(x + 2, y)), module, Some2::L_ACTIVE + i));
            addChild(createLightCentered<MediumLight<GreenLight> >(mm2px(Vec(x - 2, y)), module, Some2::L_SELECT + i));
        }
    }
};

Model *modelSome2 = createModel<Some2, Some2Widget>("Some2");