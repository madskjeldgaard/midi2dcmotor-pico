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
  - SLEEP: 12 FIXME: connected to GND currently on the board
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

motor::DRV8833 motorDriver1(0, 1, 6, 7, 2);
motor::DRV8833 motorDriver2(8, 9, 10, 11, 12);

// Set up sleep functionality
std::function<void()> enableSleepFunc = []() {
  Serial.println("Going to sleep...zzz....");
  motorDriver1.sleep();
};

std::function<void()> disableSleepFunc = []() {
  Serial.println("Waking up!");
  motorDriver1.wake();
};

AutoSleep autoSleep1(2000, enableSleepFunc, disableSleepFunc);

// Example of handling midi input to the device
void handle_midi_note_on(byte channel, byte note, byte velocity) {
  Serial.println("Got note on!");
  Serial.println(note);
  Serial.println("Channel:");
  Serial.println(channel);

  autoSleep1.disableSleep();
  autoSleep1.updateEventTime();

  motorDriver1.wake();

  auto mappedVelocity = map(velocity, 0, 127, 0, 1023);

  // TODO: Allow changing direction somehow
  const auto direction = 1;
  mappedVelocity *= direction;

  if (note == 60) {
    Serial.println("Setting speed A on motor 1:");
    Serial.println(mappedVelocity);
    motorDriver1.setSpeedA(mappedVelocity);
  } else if (note == 61) {

    Serial.println("Setting speed B on motor 1:");
    Serial.println(mappedVelocity);

    motorDriver1.setSpeedB(mappedVelocity);
  } else if (note == 62) {

    Serial.println("Setting speed A on motor 2:");
    Serial.println(mappedVelocity);

    motorDriver2.setSpeedA(mappedVelocity);
  } else if (note == 63) {

    Serial.println("Setting speed B on motor 2:");
    Serial.println(mappedVelocity);

    motorDriver2.setSpeedB(mappedVelocity);
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

  // Stop motor
  if (note == 60) {
    Serial.println("Stopping motor 1");
    motorDriver1.stopA();
  } else if (note == 61) {
    Serial.println("Stopping motor 1");
    motorDriver1.stopB();
  } else if (note == 62) {
    Serial.println("Stopping motor 2");
    motorDriver2.stopA();
  } else if (note == 63) {
    Serial.println("Stopping motor 2");
    motorDriver2.stopB();
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Hello World!");

  TinyUSBDevice.setManufacturerDescriptor("MadsKjeldgaard");
  TinyUSBDevice.setProductDescriptor("Midi2DCMotor");

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

void loop() {
  MIDI.read();
  autoSleep1.checkSleep();
}
