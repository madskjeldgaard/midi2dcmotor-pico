[![PlatformIO CI](https://github.com/madskjeldgaard/raspberry-pi-pico-usbmidi-platformio-template/actions/workflows/build.yml/badge.svg)](https://github.com/madskjeldgaard/raspberry-pi-pico-usbmidi-platformio-template/actions/workflows/build.yml)

# MIDI 2 DC Motor controller

![midi2dcmotor in action](midi2dcmotor.jpg) 

This firmware will turn a Raspberry Pi Pico with two [DRV8833 motor controller boards](https://learn.adafruit.com/adafruit-drv8833-dc-stepper-motor-driver-breakout-board) into a USB Midi to DC Motor controller of sorts.

It maps midi notes to one of the 4 currently available channels and uses the velocity of the notes to set the speeds.

These midi notes are 60-63.

It also features a small auto sleep function which will activate the DRV8833's sleep mode when idle.

## Dependencies

You need to have platformio installed to make use of this.

Install it on MacOS by running homebrew:

```bash
brew install platformio
```

## Compiling and uploading

To compile and upload your sketch, simply run the following command:

```bash
pio run -t upload
```
