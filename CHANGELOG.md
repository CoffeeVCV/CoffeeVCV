# Change Log

## v2.0.0 7 May 2022
  * Initial realease of Travel, Between, HiLo and Some.

## v2.1.0 17 May 2022
  * Adding new module 
  * Together : An 8 channel, 4 group Euclidian sequencer, with triggered randomized movement.

## v2.2.0 NN May 2022
  * Added modeule Tumble
    * A chain of up to 8 countdowns, which can produce bursts of triggers or gates.
  * Added module Set
  * Added module Fork
  * Updated module Some
    * Bug Fixes
    * Cosmetic changes
  * Updated module Together
    * Fixed bug with random
    * Added Scale and Offset for each group
    * Exnad range of dividers to 64.
    * Change mode to a switch
    * Cosmetic changes
  * Updated module Between
    * Cosmetic changes
    * Changes input calulation to source+offset*scale
  * Updated Travel
    * Cosmetic changes
    * Move option from context menu to front panel
    * Changed input calulation to source+offset*scale
  * Updated module HiLo
    * Cosmetic changes

## v2.3.0 07 July 2022
  * Added module Fork2
    * Compare an input with a threshold and output one of two other inputs.
  * Added module Some3
    * From a polyphonic input, when triggered, mute a number of the inputs, based on a probability
  * Added Juice
    * Select one of 16 sets of 8 fixed voltages, based on a CV input.
  * Added Twinned2
    * A sequencer with pair of 8 step notes, gates, randomisation and polyphonic input.
  * Fixed Tumble
    * Problems with gates

## v2.4.0 11 September 2022
  * Added module Any
    * A simple AND/OR logic utility with 8 inputs, 1 output.
  * Added module Set2
    * A single large knob with up to 4 presets and smooth interploation over a specified time.
  * Added Module Quant
    * A small quantizer.

## v2.4.1 23 October 2022
  * Fixed bug in Fork2
    * Second probabality was not working
  * Changed default in Twinned2
    * Probability os now 0.0 was 0.5
  * Changed default in Any
    * default mode is now OR, was AND

