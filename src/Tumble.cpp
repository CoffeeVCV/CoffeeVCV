#include "plugin.hpp"

#include "components.hpp"

#define NUM_ROWS 8

struct Tumble : Module
{
    enum ParamId
    {
        P_TRIG_BUTTON,
        P_RESET_BUTTON,
        P_START_BUTTON,
        P_MODE_BUTTON,
        ENUMS(P_COUNTER, NUM_ROWS),
        PARAMS_LEN
    };
    enum InputId
    {
        I_TRIG,
        I_RESET,
        I_START,
        INPUTS_LEN
    };
    enum OutputId
    {
        ENUMS(O_TRIG, NUM_ROWS),
        ENUMS(O_GATE, NUM_ROWS),
        O_ORTRIG,
        O_ORGATE,
        O_EOC,
        OUTPUTS_LEN
    };
    enum LightId
    {
        ENUMS(L_PACTIVE, NUM_ROWS),
        ENUMS(L_PTRIG, NUM_ROWS),
        ENUMS(L_PROGRESS, NUM_ROWS),
        L_START,
        LIGHTS_LEN
    };

    enum Mode
    {
        ONCE,
        LOOP
    };

    int counts[NUM_ROWS];
    int initialCount[NUM_ROWS];
    int currentCounter = 0;
    int eocRow;
    bool runState = false;
    bool modeToggleReady = true;
    int Mode;

    dsp::SchmittTrigger _clockTrigger;
    dsp::SchmittTrigger _startTrigger;
    dsp::SchmittTrigger _resetTrigger;
    dsp::BooleanTrigger _clockButtonTrigger;
    dsp::BooleanTrigger _startButtonTrigger;
    dsp::BooleanTrigger _resetButtonTrigger;
    dsp::PulseGenerator _rowPulse[NUM_ROWS];
    dsp::PulseGenerator _eocPulse;
    dsp::PulseGenerator _globalPulse;
    dsp::ClockDivider _lowPriority;

    Tumble()
    {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
        configParam(P_TRIG_BUTTON, 0.f, 1.f, 0.f, "Manual Trig");
        configInput(I_TRIG, "Clock Trig");
        configParam(P_RESET_BUTTON, 0.f, 1.f, 0.f, "Manual Reset");
        configInput(I_RESET, "Reset Trig");
        configParam(P_START_BUTTON, 0.f, 1.f, 0.f, "Manual Start");
        configInput(I_START, "Stat Trig");
        configSwitch(P_MODE_BUTTON, 0, 1, 0, "Once/Loop Mode", {"Once", "Loop"});
        for (int i = 0; i < NUM_ROWS; i++)
        {
            configParam(P_COUNTER + i, 0.f, 64.f, 0.f, string::f("Counter %d", i + 1));
            paramQuantities[P_COUNTER + i]->snapEnabled = true;
            configOutput(O_TRIG + i, string::f("Trigger %d", i + 1));
            configOutput(O_GATE + i, string::f("Gate %d", i + 1));
        }
        configOutput(O_EOC, "End of Cycle");
        configOutput(O_ORTRIG, "Global Trig");
        configOutput(O_ORGATE, "Global Gate");
        Mode = ONCE;
        reset();
        _lowPriority.setDivision(16);
    }

    void reset()
    {
        for (int i = 0; i < NUM_ROWS; i++)
        {
            initialCount[i] = params[P_COUNTER + i].getValue();
            outputs[O_TRIG + i].setVoltage(0.f);
            outputs[O_GATE + i].setVoltage(0.f);
            counts[i] = initialCount[i];
        }
        outputs[O_ORGATE].setVoltage(0.f);
        outputs[O_ORTRIG].setVoltage(0.f);
        currentCounter = 0;
    }

    void process(const ProcessArgs &args) override
    {

        bool startTriggered = _startTrigger.process(inputs[I_START].getVoltage());
        bool startButtonPressed = _startButtonTrigger.process(params[P_START_BUTTON].getValue());
        if (startTriggered || startButtonPressed)
        {
            runState = (runState) ? false : true;
            lights[L_START].setBrightness((runState) ? 1 : 0);
        }

        bool resetTriggered = _resetTrigger.process(inputs[I_RESET].getVoltage());
        bool resetButtonPressed = _resetButtonTrigger.process(params[P_RESET_BUTTON].getValue());
        if (resetTriggered || resetButtonPressed)
        {
            reset();
        }

        bool pulse = _eocPulse.process(args.sampleTime);
        outputs[O_EOC].setVoltage(pulse ? 10.f : 0.f);

        pulse = _globalPulse.process(args.sampleTime);
        outputs[O_ORTRIG].setVoltage(pulse ? 10.f : 0.f);

        bool clockTriggered = _clockTrigger.process(inputs[I_TRIG].getVoltage());
        outputs[O_ORGATE].setVoltage(_clockTrigger.isHigh() ? 10.f : 0.f);

        eocRow = -1;

        // process each row
        for (int i = 0; i < NUM_ROWS; i++)
        {
            // set the little indicator if the counter is active
            if (params[P_COUNTER + i].getValue() > 0)
            {
                lights[L_PACTIVE + i].setBrightness(1);
            }
            else
            {
                lights[L_PACTIVE + i].setBrightness(0);
            }

            // if the param changed, reset the values
            if (params[P_COUNTER + i].getValue() != initialCount[i])
            {
                initialCount[i] = params[P_COUNTER + i].getValue();
                counts[i] = initialCount[i];
            }

            // find eoc, highest numbered counter with non-zero start
            if (initialCount[i] > 0)
                eocRow = i;

            pulse = _rowPulse[i].process(args.sampleTime);
            outputs[O_TRIG + i].setVoltage(pulse ? 10.f : 0);

            if (_clockTrigger.isHigh() && currentCounter == i)
            {
                outputs[O_GATE + i].setVoltage(10.f);
            }
            else
            {
                outputs[O_GATE + i].setVoltage(0.f);
            }

            lights[L_PTRIG + i].setBrightnessSmooth(pulse ? 1 : 0, args.sampleTime);
            float v = (initialCount[i] > 0 ? float(counts[i]) / float(initialCount[i]) : 0);
            lights[L_PROGRESS + i].setBrightnessSmooth(v, args.sampleTime);
        }

        bool clockButtonPressed = _clockButtonTrigger.process(params[P_TRIG_BUTTON].getValue());
        if (runState)
        {
            if (clockTriggered || clockButtonPressed)
            {
                // process each counter
                for (int i = 0; i < NUM_ROWS; i++)
                {
                    // if this counter is not finished
                    if (counts[i] > 0)
                    {
                        // pulse
                        _rowPulse[i].trigger(1e-3f);
                        _globalPulse.trigger(1e-3f);
                        counts[i]--;
                        // if that was the last pulse, do eoc
                        if (i == eocRow && counts[i] == 0)
                        {
                            _eocPulse.trigger(1e-3f);
                            reset();
                            if (Mode == ONCE)
                            {
                                runState = false;
                                ;
                                lights[L_START].setBrightness(0);
                            }
                        }
                        currentCounter = i;
                        break; // stop processing rows
                    }
                }
            }
        }

        // low priority
        if (_lowPriority.process())
        {

            // Mode
            Mode = params[P_MODE_BUTTON].getValue();
        }
    }
};

struct TumbleWidget : ModuleWidget
{
    TumbleWidget(Tumble *module)
    {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/Tumble.svg")));

        float x, y;
        float s = 10;
        float lm = 15.12 / 2;
        float xoffset = lm;
        float yoffset = 27;

        x = xoffset;
        y = 13;
        addInput(createInputCentered<CoffeeInputPortButton>(mm2px(Vec(x, y)), module, Tumble::I_TRIG));
        addParam(createParamCentered<CoffeeTinyButton>(mm2px(Vec(x + 3.5, y - 3.5)), module, Tumble::P_TRIG_BUTTON));
        addInput(createInputCentered<CoffeeInputPortButton>(mm2px(Vec(x + s, y)), module, Tumble::I_START));
        addParam(createParamCentered<CoffeeTinyButton>(mm2px(Vec(x + s + 3.5, y - 3.5)), module, Tumble::P_START_BUTTON));
        addChild(createLightCentered<MediumLight<GreenLight> >(mm2px(Vec(x + s, y + 7)), module, Tumble::L_START));
        addInput(createInputCentered<CoffeeInputPortButton>(mm2px(Vec(x + (s * 3), y)), module, Tumble::I_RESET));
        addParam(createParamCentered<CoffeeTinyButton>(mm2px(Vec(x + (s * 3) + 3.5, y - 3.5)), module, Tumble::P_RESET_BUTTON));

        // mode
        x = x + (s * 2);
        addParam(createParamCentered<CoffeeSwitch2PosVert>(mm2px(Vec(x, y)), module, Tumble::P_MODE_BUTTON));

        x = xoffset;
        for (int i = 0; i < NUM_ROWS; i++)
        {
            y = yoffset + (s * i);
            addParam(createParamCentered<CoffeeKnob8mm>(mm2px(Vec(x, y)), module, Tumble::P_COUNTER + i));
            addChild(createLightCentered<TinyLight<YellowLight> >(mm2px(Vec(x + 6, y)), module, Tumble::L_PACTIVE + i));
            addChild(createLightCentered<MediumLight<YellowLight> >(mm2px(Vec(x + s, y - 2)), module, Tumble::L_PROGRESS + i));
            addChild(createLightCentered<MediumLight<GreenLight> >(mm2px(Vec(x + s, y + 2)), module, Tumble::L_PTRIG + i));
            addOutput(createOutputCentered<CoffeeOutputPort>(mm2px(Vec(x + (s * 2), y)), module, Tumble::O_TRIG + i));
            addOutput(createOutputCentered<CoffeeOutputPort>(mm2px(Vec(x + (s * 3), y)), module, Tumble::O_GATE + i));
        }

        x = xoffset + s;
        y = yoffset + (s * (NUM_ROWS)) + (s / 2);
        addOutput(createOutputCentered<CoffeeOutputPort>(mm2px(Vec(x, y)), module, Tumble::O_EOC));
        addOutput(createOutputCentered<CoffeeOutputPort>(mm2px(Vec(x + s, y)), module, Tumble::O_ORTRIG));
        addOutput(createOutputCentered<CoffeeOutputPort>(mm2px(Vec(x + s + s, y)), module, Tumble::O_ORGATE));
    }
};

Model *modelTumble = createModel<Tumble, TumbleWidget>("Tumble");