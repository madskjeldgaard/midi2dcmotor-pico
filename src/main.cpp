/*

Connections:
- Motor 1:
  - AIN1: 0
  - AIN2: 1
  - SLEEP: 2
  - BIN2: 7
  - BIN1: 6
- Motor 2:
  - AIN1: 7
  - AIN2: 8
  - SLEEP: ? FIXME: connected to GND currently
  - BIN2: 10
  - BIN1: 11


*/

#include "DRV8833.h"
#include <Adafruit_TinyUSB.h>
#include <Arduino.h>
#include <MIDI.h>

#include "AutoSleep.h"



// Smooth buttons and potentiometers, see the libraries for examples on how to
// use them.
#include <Bounce2.h>
#include <ResponsiveAnalogRead.h>

// USB MIDI object
Adafruit_USBD_MIDI usbMidi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usbMidi, MIDI);

motor::DRV8833 motorDriver1(0,1,6,7,2);

// Set up sleep functionality
std::function<void()> enableSleepFunc = []() {
  Serial.println("Setting sleep mode on motor 1");
  motorDriver1.sleep();
};

std::function<void()> disableSleepFunc = []() {
  Serial.println("Disabling sleep mode on motor 1");
  motorDriver1.wake();
};

AutoSleep autoSleep1(2000, enableSleepFunc, disableSleepFunc);

// Example of handling midi input to the device
void handle_midi_note_on(byte channel, byte note, byte velocity) {
  Serial.println("Got note on!");
  Serial.println(note);

  autoSleep1.disableSleep();
  autoSleep1.updateEventTime();

  motorDriver1.wake();

  // Go forward
  if (note == 60) {
    // const auto mapped = map(velocity, 0, 127, 0, 1023);
    // motorDriver1A.setSpeed(mapped);
    motorDriver1.setSpeedA(501);
  }
}

// Example
void handle_midi_note_off(byte channel, byte note, byte velocity) {
  Serial.println("Got note off!");
  Serial.println(note);


  // Sleep timer
  autoSleep1.disableSleep();
  autoSleep1.updateEventTime();
  motorDriver1.wake();

  // Go forward
  if (note == 60) {
    const auto mapped = map(velocity, 0, 127, 0, 1023);
    motorDriver1.setSpeedA(0);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Hello World!");

  TinyUSBDevice.setManufacturerDescriptor("MadsKjeldgaard");
  TinyUSBDevice.setProductDescriptor("Pico Blinkity Blinky");

  usbMidi.begin();
  MIDI.begin();

  // Turn midi thru off
  MIDI.turnThruOff();

  // Set the MIDI note on handling function
  MIDI.setHandleNoteOn(handle_midi_note_on);
  MIDI.setHandleNoteOff(handle_midi_note_off);

  autoSleep1.enableSleep();
  autoSleep1.updateEventTime();
}

void loop() { MIDI.read(); }
