/*
 *
 * This sketch sets up a Pico as a USB MIDI device that controls up to 4 DC
 * motors using the DRV8833 motor driver.
 *
 * It has autosleep functionality that puts the motor drivers to sleep after a
 * period of inactivity (to save power and silence annoying sounds in some
 * motors).
 *
 * Additionally it makes it possible to control the speed and direction of the
 * motors using MIDI notes.
 *
 * The DRV8833 driver is optimized for small motors, see comments below.
 *
 */
#include "AutoSleep.h"
#include "DRV8833.h"

#include <Adafruit_TinyUSB.h>
#include <Arduino.h>
#include <MIDI.h>

// These are optimizations to increase performance of small DC motors.
// Adjust to your liking, see this article for additional info:
// https://learn.adafruit.com/improve-brushed-dc-motor-performance/pwm-frequency
constexpr auto PWM_FREQ = 100; // HZ
const auto DECAY_MODE = motor::DecayMode::Slow;

// USB MIDI object
Adafruit_USBD_MIDI usbMidi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usbMidi, MIDI);

// Pins of the motor drivers
constexpr auto motor1_A1 = 0, motor1_A2 = 1, motor1_sleep = 2, motor1_B2 = 7,
               motor1_B1 = 6;
motor::DRV8833 motorDriver1(motor1_A1, motor1_A2, motor1_B1, motor1_B2,
                            motor1_sleep, DECAY_MODE);

constexpr auto motor2_A1 = 8, motor2_A2 = 9, motor2_sleep = 12, motor2_B2 = 10,
               motor2_B1 = 11;
motor::DRV8833 motorDriver2(motor2_A1, motor2_A2, motor2_B1, motor2_B2,
                            motor2_sleep, DECAY_MODE);

// Set up sleep functionality
std::function<void()> enableSleepFunc = []() {
  Serial.println("Going to sleep...zzz....");
  motorDriver1.sleep();
};

std::function<void()> disableSleepFunc = []() {
  Serial.println("Waking up!");
  motorDriver1.wake();
};

constexpr auto sleepTresholdMS = 5000;
AutoSleep autoSleep1(sleepTresholdMS, enableSleepFunc, disableSleepFunc);

void eventHappened() {
  autoSleep1.disableSleep();
  autoSleep1.updateEventTime();
  motorDriver1.wake();
  motorDriver2.wake();
}

// Example of handling midi input to the device
void handle_midi_note_on(byte channel, byte note, byte velocity) {
  Serial.println("Got note on!");
  Serial.println(note);
  Serial.println("Channel:");
  Serial.println(channel);

  eventHappened();
  auto mappedVelocity = map(velocity, 0, 127, 0, 1023);

  // Midi notes 60-63 are used to turn on the motors in forward mode
  // 60: Motor 1 A
  // 61: Motor 1 B
  // 62: Motor 2 A
  // 63: Motor 2 B
  //
  // The velocity of the note is used to set the speed of the motor
  //
  // Midi notes 48-51 are used to turn on the motors in reverse mode
  // 48: Motor 1 A
  // 49: Motor 1 B
  // 50: Motor 2 A
  // 51: Motor 2 B
  if (note == 48) {
    motorDriver1.getBridgeA().setSpeed(mappedVelocity);
  } else if (note == 49) {
    motorDriver1.getBridgeB().setSpeed(mappedVelocity);
  } else if (note == 50) {
    motorDriver2.getBridgeA().setSpeed(mappedVelocity);
  } else if (note == 51) {
    motorDriver1.getBridgeB().setSpeed(mappedVelocity);
  } else if (note == 60) {
    motorDriver1.getBridgeA().setSpeed(mappedVelocity);
  } else if (note == 61) {
    motorDriver1.getBridgeB().setSpeed(mappedVelocity);
  } else if (note == 62) {
    motorDriver2.getBridgeA().setSpeed(mappedVelocity);
  } else if (note == 63) {
    motorDriver1.getBridgeB().setSpeed(mappedVelocity);
  }
}

// Example
void handle_midi_note_off(byte channel, byte note, byte velocity) {
  Serial.println("Got note off!");
  Serial.println(note);

  eventHappened();

  // Stop motor
  if (note == 48) {
    motorDriver1.getBridgeA().stop();
  } else if (note == 49) {
    motorDriver1.getBridgeB().stop();
  } else if (note == 50) {
    motorDriver2.getBridgeA().stop();
  } else if (note == 51) {
    motorDriver2.getBridgeB().stop();
  } else if (note == 60) {
    motorDriver1.getBridgeA().stop();
  } else if (note == 61) {
    motorDriver1.getBridgeB().stop();
  } else if (note == 62) {
    motorDriver2.getBridgeA().stop();
  } else if (note == 63) {
    motorDriver2.getBridgeB().stop();
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Hello World!");

  analogWriteFreq(PWM_FREQ);

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
