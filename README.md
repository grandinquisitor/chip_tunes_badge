# Chip tunes badge

The goal of this project is to play a relatively long ringtone at a relatively loud volume using only a weak coin cell battery and inexpensive components, including an inexpensive microcontroller (ATtiny13a).

## The above paragraph in list format

* Loud
* Cheap
* Coin cell battery

## Other features

There's also a single LED that animates according to the music. This adds a small but significant dimension of fun.

## Platform

The code does not use any special hardware features such as hardware PWM on the microcontroller except for the timer needed for the `millis()` function, so it should be easy to port or generalize to other microcontrollers.

## Code size and music format

Despite the ATtiny13 we are targeting having only 1k of flash program space, we are able to fit the first full 12 measures of the Imperial March. In order to do this, instruction size needs to be kept to an absolute minimum and the tune needs to be converted from RTTTL to a much more compact format using python. It may be possible to come up with an even more compact format.

The LED animation was designed to take as little space as possible and just reflects the duration of the current note in brightness.

## Circuit design

Normally you can drive a piezo off of one microcontroller pin. The results may vary however, and will probably not be loud enough to compete with any other noise in the room.

Remember, a piezo buzzer is like a capacitor: current doesn't really flow through it, what happens is the piezo disc physically deforms according to a voltage differential across it. So voltage makes it louder without adding much current.

The next step is to drive the piezo off of *two* microcontroller pins that generate an AC square wave. Rather than one pin of the piezo being high or low and the other pin being ground, instead each side alternates high and low on separate microcontroller pins. Using two pins on the same port register we can swap them virtually simultaneously. This "doubles" the effective amplitude of the voltage of the square wave.

But how can we make it even louder? Using a really basic charge pump square wave voltage doubler circuit. This further doubles the voltage across the piezo, minus the voltage drop of the diodes. That's the design used here. You should be able to chain more voltage doublers in series to keep making it louder, in theory.

## Choice of microcontroller

The ATtiny13a has an ultra-low power sleep mode, and is low cost. Except for the sleep/interrupt no special hardware functions are used, so it should be possible to support other microcontrollers that have 1k of flash space or above.

## Interface

Only a single button is used. It wakes the microcontroller, plays the song, then goes back to sleep. When the circuit is not active, the microcontroller is kept in ultra low power sleep. This makes a physical power off switch not strictly necessary.

Depressing the button a second time while the tune is being played will cancel the song in progress.

## Changing the tune

Edit `convert.py` python script with a new [RTTTL](https://en.wikipedia.org/wiki/Ring_Tone_Transfer_Language) ringtone, generate the more compact song output, and paste the result into the C code.

## Inspiration/Prior art

* [TimerFreeTone](https://bitbucket.org/teckel12/arduino-timer-free-tone/wiki/Home)
* [ToneAC](https://bitbucket.org/teckel12/arduino-toneac/wiki/Home)
