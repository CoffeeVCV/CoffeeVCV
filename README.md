# CoffeeVCV Modules

![Coffee VCV Family](images/Coffee-Family1.png)

Modules for [VCV Rack](https://github.com/VCVRack/Rack), an open-source Eurorack-style virtual modular synthesizer:
  - [Set2](#Set2)
  - [Any](#Any)
  - [Quant](#Quant)
  - [Twinned2](#Twinned2)
  - [Juice](#Juice)
  - [Some3](#Some3)
  - [Fork2](#fork2)
  - [Tumble](#Tumble)
  - [Together](#Together)
  - [Travel](#Travel)
  - [Between](#Between)
  - [HiLo](#HiLo)
  - [Some](#Some)
  - [Some 2](#Some2) 
  - [Set](#Set)
  - [Tap](#Tap)
  - [Fork](#Fork)


---
![V2.4](images/v2.4-Group.png)
## v2.4.0 11 September 2022
  * Added module Any
    * A simple AND/OR logic utility with 8 inputs, 1 output.
  * Added module Set2
    * A single large knob with up to 4 presets and smooth interploation over a specified time.
  * Added Module Quant
    * A small quantizer.
  
---
## <a name="Set2"></a> Set2
![Set2 panel](images/Set2-Panel.png)

### Overview

### Features

### Basic Operation

---
## <a name="Any"></a> Any
![Any panel](images/Any-Panel.png)

### Overview

### Features

### Basic Operation

---
## <a name="Quant"></a> Quant
![Quant panel](images/Quant-Panel.png)

### Overview

### Features

### Basic Operation

---
## <a name="Twinned2"></a> Twinned2
![Tumble panel](images/Twinned2-Panel.png)

### Overview
Twinned2 is a sequencer with a pair of 8 step note sequences.  Each step has a probability, and that determines which sequennce the played note comes from.  Both sequences also have variable gates.  The longest gate time is one whole beat (100%), the shortest is 0%, essentailly muted.  The beat is determined by the inpute clock.

### Features
* Clock input with manual trigger
* Reset input with manual trigger
* Two separate 8 step sequences (A/B)each with
  * 8 knobs for notes (0 to 1v)
  * 8 knobs for octave (-10 to 10v)
  * 8 knobs gates (0 to 100%)
  * Input to provide 8 channel polyphonic notes
  * Input and manual trigger for note randomisation (randomisation amout is manually contolable)
  * Input and manual trigger for gate duration randomisation
  * v/oct output
  * polyphonic gate output
* v/oct out controled by probability
* polyphonic gate output controlled by probability
* End of cycle trigger
* 8 knobs to control probability between sequence A and B
* Input to control sequence slection
* Knob to control selection threshold
* Steps input to set number of steps
* Steps know to set number of steps
* Button for randomizing groups of controlls
* Knob to control amount of randomization
* Context Menus for
  * Polyphonic/Monophonic gate output
  * Control visual updates
  * Copy notes or gates between sequences
  * Control randomisation
  * Control voltage scales


### Basic Operation

#### __Clock and Reset__
Connect a clock and reset, or use the manual button on the top right of the inputs.

#### __Notes, Octaves and Gates__
The module is roughly mirrored from left to right, down the centre column of knobs (probability).

The far left column control the gates for sequence A.  The range for gates is 0% to 100%, which represents the duration between clock triggers.  100% is fully open for the whole duration.

The next two columns of knobs are octave and note.  The smaller knob is octave (-10v to 10v), the larger is note (0v to 1v), these are summed to produce pitch for that step of sequence A.

Sequence B is reversed, far right is gate.

#### __Probability__
The centre column of knobs is probability.  The range is 0v to 1v.  For each step probability is assessed, pitch and gate will be from the winning sequence.  An indicator light will flash showing which sequence was selected.  Setting probability to 0v will always select sequence A, 1v will always select sequence B.  0.5v is essentially a 50/50 coin toss.

#### __Pitch Output__
Accross the bottom of the module are three outputs.  The one on the left is dedicated to sequence A, the one of the right is for sequence B.  The centre output is for pitch selected by probability.

#### __Gate output and End of Cycle__
On the right of the module are four outputs arranged virtically.  From the top, here's their purpose.
* Sequence A Gates
* Sequence B Gates
* Gates selected by probability
* End of Cycle trigger

Gate outout is 0v or 10v.
If gate polyphony is selected in the context menu, then 8 channels per output is produced.

#### __Polyphonic input__
For both sequence, gates and pitch can be provided by 8 channel polyphony, to the inputs directly above the respective knob columns.
By daults knob controls will be visually updted to reflect polyphonic input.  This can be adjusted in the context menu.

#### __Sequence length__
The default number of steps is 8.
To set an alternate length, either use the knob on the far left called 'Steps', or provide positive voltage to 'Steps Input'.  The default range os 0v to 10v., this can be changed in the context menu, to 0v-1v or 0v-8v.  If the provided voltage is out of range, a red led will show on the Steps Input.

---
## <a name="Juice"></a> Juice
![Juice panel](images/Juice-Panel.png)

### Overview

### Features

### Basic Operation

---
## <a name="Some3"></a> Some3
![Some3 panel](images/Some3-Panel.png)

### Overview

### Features

### Basic Operation

---
## <a name="Fork2"></a> Fork2
![Fork2 panel](images/Fork2-Panel.png)

### Overview

### Features

### Basic Operation

---
## <a name="Tumble"></a> Tumble
![Tumble panel](images/Tumble-Panel.png)

### Overview
Tumble, provides chain of up to 8 pulsing countdowns, which can produce bursts of triggers or gates.  This can be used as a sequencer, or a pulse generator.
Tumble pairs well with other logic modules.

### Features
* Clock \ Trigger input with manual button
* Start trigger input with manual button and indicator light
* Reset trigger input with manual button
* Once \ Loop mode switch
* 8 Countdowns with a range of 1 - 64, each with
  * Trigger and Gate output
  * Activity light
  * Indicator lights
* Global trigger outputs
* Global gate output
* End of Cycle Trigger output

### Basic Operation
1. Connect a clock to clock
2. Set some counters to non zero values.
3. Press or trigger Start

Once started, on each clock pulse the first counter will start to countdown.
Counters are prioritized, from top to bottom.
Once a counter reaches zero, the next counter will start to countdown.
Each time a counter decreases, a pulse is sent to it's trigger and gate, and also the global trigger and gate.
On the last count of the last counter, the End Of Cycle trigger will pulse.
If the mode swicth is set ot Once, another start trigger is needed to restart the process.  

If the mode it set to loop, it will start automatically on the next clock pulse.

In the following example there are two counters configured.
* Counter #1 set to 3 steps (shown as red on scope)
* Counter #2 set to 1 (orange on scope)

We can see the triggers in red and orange in the scope.

Global gate is the green scope.
Global trig and End of Cycle trig are shown on the left scope, in purple and blue.


![Together panel](images/Tumble-Example.png)



---
## <a name="Together"></a> Together
![Together panel](images/Together-Panel.png)

### Overview
Together, is a fun Euclidian sequencer which provides a bank of dividers, resulting in triggers and gates, and 4 CV outputs.

### Features
* Clock \ Trigger input
* Reset input with manual
* 8 Clock dividers 
* 8 outputs as trigger and gates
* Global trigger output
* Global gate output
* 4 columns of adjustable values
* 4 output CVs
* Global CV output scaling
* Global CV output offset
* Randomization of dividers
* Randomization of CV values
* Randomization of tigger outs
* CV Inputs with manual triggers

### Clock and Reset
At the top right of the module, there are two inputs, one to supply the module with a clock or trigger, the other is to recive a reset signal.  In the top right of each input control there a small button that can be used to manually trigger input.

This image show the clock connected and active, and reset is not connected.

![Dividers](images/Together-Clock-Reset.png)


### Clock Dividers / Steps
The large knobs on the left configure the dividers which can be set to trigger every 1 - 16 steps. 
Each divider has three indicators.
* Large yellow light - the step is triggering
* Smallest yellow light - the divider is active (set > 0)
* Orange light - divider is locked, and will not be changed by triggered adjustment.

This image show three dividers.
1. Not configured.
2. Configured and triggering.
3. Locked, configured and not triggering.

![Dividers](images/Together-Divider1.png)


### Divider output
In it's simplest form this sequencer will provide trigger or gate output.  
These are located on the far right, each divider row also has a probability control.  When probability is set to 1.0, output will always occur when a step activates.  A divider with a setting of 0.5 will output 50% of the time, 0.0 will never produce output.

Small red and green lights, indicate if the probability blocked or allowed output.

The image below shows the output for three dividers.  
1. Probability 100%, trigger is firing and connected.
2. Probability 100%, gate is active.
3. Probability is low, and trigger is not firing.

![DividerOutput](images/Together-TrigOutput1.png)


### Global Divider Output
In the bottom right of the module, there two other outputs which will always be active if any output in their column is active.

If a divider doesn't activate beacuse probability has blocked it, the global output will not fire unless a different divider is active, and not blocked.

This image shows the global trigger not firing, and the global gate is active.

![TogetherGlobalTriggerOutput](images/Together-GlobalOutput1.png)


### Group CV columns and output
In the centre there's matrix if 4 x 8 CV adjustment knobs.  The range for each is -1.0v to 1.0v.  At the bottom of the columns there are 4 CV outputs.

![4 x 8 Outputs](images/Together-4x8Outputs.png)

When the clock triggers the dividers steps, the CV value are combined and shared via the column output.
Only values for triggering rows are included in the result.

In this image there are 4 CV settings, and diverders are set to trigger on steps 5,4,3 and 2.  The scope shows the values combining as the dividers trigger, creating an interesting rhythem.
![Together-ColumnCVExample1.png](images/Together-ColumnCVExample1.png)

There are 4 of these columns, each operate independantly.

### Scale and Offset
At the bottom left of the module are two adjust ment knobs, which alter the final results of all the column outputs.  Both have a range of -5.0v to +5.0v.
Scale multiplies the output. 
By default the scale is set to 1, if it is set to 2, results will be doubled.
Offset adds or subtracts a value from the result, the default is 0.0v.

![Together-Scale-Offset](images/Together-ScaleOffset1.png)

### Nudge and Random
This sequencer is able to small or large changes to the divider step setting, and the 4x8 matrix.  There are two modes, which can be applied to various groups of controls.

Modes
* Random - The controls value is replaced with a random value, in the range of the control.
* Nudge - The controls value is replaced with a modified values, based on the current value.

Groups
* All the dividers, except the locked dividers.
* Each column of CV controls
* Each row of CV controls
* All the CV controls

The top row of the module has the controls to adjust dividers and groups.  Most are CV Inputs with manul triggers.
![Together-NudgeRND](images/Together-StepNudgeRND.png)

From left to right the controls are.

1. Divider adjust
2. Mode 
3. Group 1 adjust
4. Group 2 adjust
5. Group 3 adjust
6. Group 4 Adjust
7. All groups and row adjust
8. Adjustment limit

#### Divider Adjust
Trigger will change all the dividers that are not locked.
* Random 0 - 16
* Nudge -1 or +1

#### Mode
This is push button, with two indicators.
Activating this with toggle between Nudge and Random
Yellow at the top is Nudge Mode.
Yellow at the bottom is Random.

#### Groups 1 - 4
Each of these will replace all the CV values with an adjusted value.
* Random 0.0v - 1.0v
* Nudge +/- the value of [Adjustment Limit](#AdjustmentLimit)

#### All rows and columns 
This will adjust every control in the 4 x 8 matrix.

#### <a name="Adjustment Limit">Adjustment Limit</a>
This control set the amount that will be added to, or substracted from CV control, when mode is Nudge.  The default is 0.05v.

#### Row Adjustment 
To the immediate right of column 4, is the triggers for each row.  These operate in the same way as columns.

![Together-NudgeRND](images/Together-RowNudge.png)

---

## <a name="Travel"></a> Travel
![Travel panel](images/Travel-Panel.png)

### Overview
Travel take two values and outputs an interpolation over a defined period of time.

The two values are provided via inputs In1 and In2 which are modified by Scale and Offset.
If no input if provided the default value is 0v, which cn be modified by Offset.

The cycle is started either the manual or CV Trig input.
The cycle stops when the duration time it met.
Duration is set manaully or via CV.
The default scale for duration is 1 and can be set to 1, 10 or 100.
A CV input to duration of 2v, with scale set to 1, will set a 2 second duration.

During the cycle the LED will indicate the cycle position, brightest at the start, and dims to off when the cycle completes. 

At the end of the cycle the EoC Output will pulse. Connecting the EOC to the input trigger will create a repeating cycle.

In the context menu, there's an option to select either Track or 
Hold.  The default is Hold, this fixes the input values until the end of the cycle.
Selecting Track will enable the input values to change during the cycle. 

The Shape CV and parameter, control the shape of the interpollation.  The default shape is linear.  -5v and 5v will changen the rate of interpolation using an expotential function.

In these images, orange is the interpolated value, and blue is the EoC pulse.

![Shape 0v Linear](images/Travel%20Linear.png)

![Shape 0v Linear](images/Travel%20Shape%20-5.png)

![Shape 0v Linear](images/Travel%20Shape%205.png)

---
## <a name="Between"></a> Between
![Travel panel](images/Between-Panel.png)

### Overview
Between will provide a random value, limited by two input values.

Output is set when either trigger manually or via clock or trigger input.

The limit values are either set via trimpots or provided as inputs to In1 or In2.
In either case, they are adjusted by Offset.

Manually setting CV1 to +5.0 and offset to -5v, will result in 0v.

In this example, green is a 0v reference, and blue is output from Random, being triggered by a clock.

In1 = -5, In2 = 5

![Between Scope](images/BetweenScope.png)

---
## <a name="HiLo"></a> HiLo
![HiLo panel](images/HiLo-Panel.png)

### Overview
HiLo takes two inputs and outputs the highest and lowest value of either input.

Output can be set via manaul or CV trigger.

In1 and In2 are each first adjusted via Scale and then Offset.
If no CV input is provided, Offset can be used to set a value (-5v to 5v).

In the context menu, there's an option to select either Track or 
Hold.  The default is Hold, this mode requires Trigger to be activated to take a sample.
In Track mode, inputs are continually sampled, but only if there's no Trigger CV connected.

Here's an example of two overlapping wave inputs, and the resulting histest and lowest values.

![HiLo Scope](images/HiLo%20Scope.png)

---
## <a name="Some"></a> Some
![Some panel](images/Some-Panel.png)

### Overview
The Some module accepts up to 8 inputs, and will output 8 or less depending on a Probability parameter.
The selection is randomised and made using the manual or CV trigger.
Only connected inputs are included in the selection process.  Selected inpute are indicated with an illuminated green led.

In this example all 8 are connected, Probability is set to 0.5.
Only 4 inputs are passed through.  When triggered again, a different 4 will be selected.

![Some Example 1](images/Some-Example1.png)

In this example with only 4 connected, Probability is still 0.5, so only 2 inpute are selected.

![Some Example 2](images/Some-Example2.png)

---
## <a name="Some 2"></a> Some 2
![Some 2 panel](images/Some2-Panel.png)

### Overview
The Some2 module is a utility module which can be used to send a single source input to up to 8 randomly selected outputs.

### Features
* Input Trigger with manual button
* Source Input CV
* Select knob
* Select Input CV (range 0 to 8, 1v per output)
* Probability knob
* Probability Input CV 0v to 1v
* 8 x Outpout CV
* 8 x Selection indicators
* 8 x Active indicators

### Operation
Connect a input source to the the input CV.
Connect some outputs
Set the selection knob, and the probability.
Trigger the function.

Each time it's triggered, a number of the selected outputs will be set to the input CV, and the others will be set to 0v.  If probabilioty is set to 0.5v, then half of the selected outputs will be active, half inactive. 
If Select is less that 8, then the first n outputs are in scope for selection.

In the follwoing example, the inpout CV is 2v (Green)
Clock is connected via Blue.
Select is set to 6v which selects 6 of the 8 outputs, shown by 6 selection indicators.

Probability is set to 0.5v manually using the knob.
Each time clock trigger, 3 outputs are activated, chossen randomly from the 6 selected.

![Some2 Example 1](images/Some2-Example1.png)

---
## <a name="Set"></a> Set
![Set panel](images/Set-Panel.png)

### Overview
Set, is a dual VCA module, which accepts an CV input, applys an offset, some scaling and outputs the result.

The offset and scaling con be controlled via CV.

### Features
* Two independant functions, each with :
  * Input CV
  * Offest -5V to 5V
  * CV Offset input
  * Scaling -x5 to x5
  * CV Scaling Input
  * CV Output

In the example below the supply CV is shown in red, and the altered voltage is shown in orange.

![Set Example 1](images/Set-Example1.png)

---
## <a name="Tap"></a> Tap
![Tap panel](images/Tap-Panel.png)

### Overview
Tap, is a simple utility module which provides three manual push buttons which each provide a trigger pule and a gate output.

### Features
* Three independant functions, each with :
  * Bush Button
  * Trigger Output 10v
  * Gate Output 10v

The gate will be held open as long as the button is activated.

In the example below the button has been pressed a few times, and held for different durations.

![Tap Example 1](images/Tap-Example1.png)

---
## <a name="Fork"></a> Fork
![Fork panel](images/Fork-Panel.png)

### Overview
Fork is a utility for making random choices.

### Features
* Two independant function each with :
  * Input trigger with manual button.
  * Probabilty control -1 to +1
    * -1 all A
    * 0 50/50 chance
    * +1 all B
  * CV Probabality Input
  * 2 x Inputs with indicators.
  * Output CV 

Provide two inputs A and B, and set the probability.  When triggered the winning input will be sampled and sent to output.

Output value will remain until triggered again.

Triggering acan be via input, or manual.

Probabaility can be via CV or manually set.

An indicator light show which input was successful.

In the example below, a trigger is provided (green), the probability is manually set to 50/50, and the input CV is (oragen and purple) being switch to produce the output shown in blue on the scope.

![Fork Example 1](images/Fork-Example1.png)