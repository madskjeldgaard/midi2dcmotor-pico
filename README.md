[![PlatformIO CI](https://github.com/madskjeldgaard/raspberry-pi-pico-usbmidi-platformio-template/actions/workflows/build.yml/badge.svg)](https://github.com/madskjeldgaard/raspberry-pi-pico-usbmidi-platformio-template/actions/workflows/build.yml)

# MIDI 2 DC Motor controller

This firmware will turn a Raspberry Pi Pico with two DRV8833 controller boards into a USB Midi to DC Motor controller of sorts.

It maps midi notes to one of the 4 currently available channels and uses the velocity of the notes to set the speeds.

These midi notes are 60-63.

It also features a small auto sleep function which will activate the DRV8833's sleep mode when idle.

## Features

- Platformio-based – easy to compile and upload, gets dependencies automatically
- Has common libraries set up:
  - Bounce2 for debouncing button presses
  - ResponsiveAnalogRead for smoother readings of analog voltages / potentiometers
  - Adafruit MIDI library
  - TINYUSB for USB Midi
- C++17, allowing a lot of modern C++ tricks.
- VSCode tasks are included making building easy in VSCode and in NeoVim (using the Overseer plugin)
- A Github Action which runs every time you push code to test if your firmware still compiles.

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
