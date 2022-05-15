# CoffeeVCV Modules

Modules for [VCV Rack](https://github.com/VCVRack/Rack), an open-source Eurorack-style virtual modular synthesizer:

  - [Travel](#Travel)
  - [Between](#Between)
  - [HiLo](#HiLo)
  - [Some](#Some)
  

## <a name="Travel"></a> Travel
![Travel panel](images/Travel1.png)

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


## <a name="Between"></a> Between
![Travel panel](images/Between.png)

Between will provide a random value, limited by two input values.

Output is set when either trigger manually or via clock or trigger input.

The limit values are either set via trimpots or provided as inputs to In1 or In2.
In either case, they are adjusted by Offset.

Manually setting CV1 to +5.0 and offset to -5v, will result in 0v.

In this example, green is a 0v reference, and blue is output from Random, being triggered by a clock.

In1 = -5, In2 = 5

![Between Scope](images/BetweenScope.png)


## <a name="HiLo"></a> HiLo
![HiLo panel](images/HiLo.png)

HiLo takes two inputs and outputs the highest and lowest value of either input.

Output can be set via manaul or CV trigger.

In1 and In2 are each first adjusted via Scale and then Offset.
If no CV input is provided, Offset can be used to set a value (-5v to 5v).

In the context menu, there's an option to select either Track or 
Hold.  The default is Hold, this mode requires Trigger to be activated to take a sample.
In Track mode, inputs are continually sampled, but only if there's no Trigger CV connected.

Here's an example of two overlapping wave inputs, and the resulting histest and lowest values.

![HiLo Scope](images/HiLo%20Scope.png)

## <a name="Some"></a> Some
![Some panel](images/Some-Panel.png)

The Some module accepts up to 8 inputs, and will output 8 or less depending on a Probability parameter.
The selection is randomised and made using the manual or CV trigger.
Only connected inputs are included in the selection process.  Selected inpute are indicated with an illuminated green led.

In this example all 8 are connected, Probability is set to 0.5.
Only 4 inputs are passed through.  When triggered again, a different 4 will be selected.

![Some panel](images/Some-Example1.png)

In this example with only 4 connected, Probability is still 0.5, so only 2 inpute are selected.

![Some panel](images/Some-Example2.png)

