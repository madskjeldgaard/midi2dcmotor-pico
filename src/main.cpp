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
 * -----------------------------------------------------------
 *
 *  TODO:
 *  - Rewrite sleep functionality to use the TaskScheduler library
 *  - Rewrite midi handling to use the generalized motor control interface
 */
#include "AutoSleep.h"
#include "DRV8833.h"
#include <SerialCommands.h>

#include <Adafruit_TinyUSB.h>
#include <Arduino.h>
#include <MIDI.h>

// These are optimizations to increase performance of small DC motors.
// Adjust to your liking, see this article for additional info:
// https://learn.adafruit.com/improve-brushed-dc-motor-performance/pwm-frequency
constexpr auto PWM_FREQ = 100; // HZ
const auto DECAY_MODE = motor::DecayMode::Slow;

// Pico art bot pins

#define PICOARTBOT1
#ifdef PICOARTBOT1

constexpr auto motor1_A1 = 8, motor1_A2 = 9, motor1_sleep = 6, motor1_B2 = 11,
               motor1_B1 = 10;

constexpr auto motor2_A1 = 12, motor2_A2 = 13, motor2_sleep = 7, motor2_B2 = 15,
               motor2_B1 = 14;

#else
constexpr auto motor1_A1 = 0, motor1_A2 = 1, motor1_sleep = 2, motor1_B2 = 7,
               motor1_B1 = 6;
constexpr auto motor2_A1 = 8, motor2_A2 = 9, motor2_sleep = 12, motor2_B2 = 10,
               motor2_B1 = 11;
#endif

motor::DRV8833 motorDriver1(motor1_A1, motor1_A2, motor1_B1, motor1_B2,
                            motor1_sleep, DECAY_MODE);

motor::DRV8833 motorDriver2(motor2_A1, motor2_A2, motor2_B1, motor2_B2,
                            motor2_sleep, DECAY_MODE);
// ----------------------------------------------------------------
// Motor control interface
// ----------------------------------------------------------------

// Function to set motor speed
void setMotorSpeed(int motorNum, char bridge, float speed) {
  Serial.println("Setting motor speed");
  Serial.println("Motor:");
  Serial.println(motorNum);

  Serial.println("Bridge:");
  Serial.println(bridge);

  Serial.println("Speed:");
  Serial.println(speed);

  if (motorNum == 1) {
    if (bridge == 'a') {
      motorDriver1.getBridgeA().setSpeed(speed);
    } else if (bridge == 'b') {
      motorDriver1.getBridgeB().setSpeed(speed);
    }
  } else if (motorNum == 2) {
    if (bridge == 'a') {
      motorDriver2.getBridgeA().setSpeed(speed);
    } else if (bridge == 'b') {
      motorDriver2.getBridgeB().setSpeed(speed);
    }
  } else {
    Serial.println("Motor number not recognized");
  }
}

// TODO:
// Function to set motor direction
// void setMotorDirection(int motorNum, char bridge, String direction) {
//   if (motorNum == 1) {
//     if (bridge == 'a') {
//       motorDriver1.getBridgeA().setDirection(direction == "forward"
//                                                  ? motor::Direction::Forward
//                                                  :
//                                                  motor::Direction::Reverse);
//     } else if (bridge == 'b') {
//       motorDriver1.getBridgeB().setDirection(direction == "forward"
//                                                  ? motor::Direction::Forward
//                                                  :
//                                                  motor::Direction::Reverse);
//     }
//   } else if (motorNum == 2) {
//     if (bridge == 'a') {
//       motorDriver2.getBridgeA().setDirection(direction == "forward"
//                                                  ? motor::Direction::Forward
//                                                  :
//                                                  motor::Direction::Reverse);
//     } else if (bridge == 'b') {
//       motorDriver2.getBridgeB().setDirection(direction == "forward"
//                                                  ? motor::Direction::Forward
//                                                  :
//                                                  motor::Direction::Reverse);
//     }
//   }
// }

// Function to stop motor
void stopMotor(int motorNum, char bridge) {
  if (motorNum == 1) {
    if (bridge == 'a') {
      motorDriver1.getBridgeA().stop();
    } else if (bridge == 'b') {
      motorDriver1.getBridgeB().stop();
    }
  } else if (motorNum == 2) {
    if (bridge == 'a') {
      motorDriver2.getBridgeA().stop();
    } else if (bridge == 'b') {
      motorDriver2.getBridgeB().stop();
    }
  }
}

void setDecayMode(int motorNum, char bridge, String newMode) {
  const auto decayMode =
      newMode == "fast" ? motor::DecayMode::Fast : motor::DecayMode::Slow;

  if (motorNum == 1) {
    if (bridge == 'a') {
      motorDriver1.getBridgeA().setDecayMode(decayMode);
    } else if (bridge == 'b') {
      motorDriver1.getBridgeB().setDecayMode(decayMode);
    }
  } else if (motorNum == 2) {
    if (bridge == 'a') {
      motorDriver2.getBridgeA().setDecayMode(decayMode);
    } else if (bridge == 'b') {
      motorDriver2.getBridgeB().setDecayMode(decayMode);
    }
  }
}

// Function to put motors to sleep
void sleepMotors() {
  motorDriver1.sleep();
  motorDriver2.sleep();
}

// ----------------------------------------------------------------
// Auto sleep
// ----------------------------------------------------------------

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

// ----------------------------------------------------------------
// USB MIDI
// ----------------------------------------------------------------

// USB MIDI object
Adafruit_USBD_MIDI usbMidi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usbMidi, MIDI);

void handle_midi_note_on(byte channel, byte note, byte velocity) {
  Serial.println("Got note on!");
  Serial.println(note);
  Serial.println("Channel:");
  Serial.println(channel);

  eventHappened();
  const auto newSpeed = static_cast<float>(velocity) / 127.0f;

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
    motorDriver1.getBridgeA().setSpeed(newSpeed);
  } else if (note == 49) {
    motorDriver1.getBridgeB().setSpeed(newSpeed);
  } else if (note == 50) {
    motorDriver2.getBridgeA().setSpeed(newSpeed);
  } else if (note == 51) {
    motorDriver1.getBridgeB().setSpeed(newSpeed);
  } else if (note == 60) {
    motorDriver1.getBridgeA().setSpeed(newSpeed);
  } else if (note == 61) {
    motorDriver1.getBridgeB().setSpeed(newSpeed);
  } else if (note == 62) {
    motorDriver2.getBridgeA().setSpeed(newSpeed);
  } else if (note == 63) {
    motorDriver1.getBridgeB().setSpeed(newSpeed);
  }
}

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

// ----------------------------------------------------------------
// Serial command interface
// ----------------------------------------------------------------

char serial_command_buffer_[32];
SerialCommands serialCommands(&Serial, serial_command_buffer_,
                              sizeof(serial_command_buffer_), "\r\n", " ");
// This is the default handler, and gets called when no other command matches.
//  Note: It does not get called for one_key commands that do not match
void cmdUnrecognized(SerialCommands *sender, const char *cmd) {
  sender->GetSerial()->print("Unrecognized command [");
  sender->GetSerial()->print(cmd);
  sender->GetSerial()->println("]");

  sender->GetSerial()->println("---------------");

  sender->GetSerial()->println("Available commands:");
  sender->GetSerial()->println("set <motor> <property> <value>");
  sender->GetSerial()->println("  motor: 1a, 1b, 2a, 2b");
  sender->GetSerial()->println("  property: speed, stop, sleep, decay");
  sender->GetSerial()->println("  value: float, string");
}

// Serial command to set motor properties
void cmdSetMotor(SerialCommands *sender) {

  String motor = sender->Next();
  String property = sender->Next();
  String value = sender->Next();

  sender->GetSerial()->println("Setting motor properties");

  sender->GetSerial()->println("Motor:");
  sender->GetSerial()->println(motor);
  sender->GetSerial()->println("Property:");
  sender->GetSerial()->println(property);
  sender->GetSerial()->println("Value:");
  sender->GetSerial()->println(value);

  eventHappened();

  int motorNum = motor.charAt(0) - '0';
  char bridge = motor.charAt(1);

  if (property == "speed") {
    sender->GetSerial()->print("Setting speed: ");
    sender->GetSerial()->println(value.toFloat());

    setMotorSpeed(motorNum, bridge, static_cast<float>(value.toFloat()));
    // } else if (property == "dir") {
    //   setMotorDirection(motorNum, bridge, value);
  } else if (property == "stop") {
    stopMotor(motorNum, bridge);
  } else if (property == "sleep") {
    sleepMotors();
  } else if (property == "decay") {
    setDecayMode(motorNum, bridge, value);
  }
}

SerialCommand cmdSet_command("set", cmdSetMotor);

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

  serialCommands.AddCommand(&cmdSet_command);
  serialCommands.SetDefaultHandler(cmdUnrecognized);

  // Wake all motors
  motorDriver1.wake();
  motorDriver2.wake();

  setMotorSpeed(1, 'a', 0.1);
  setMotorSpeed(1, 'b', 0.1);
  setMotorSpeed(2, 'a', 0.1);
  setMotorSpeed(2, 'b', 0.1);
}

void loop() {
  MIDI.read();
  // Process serial commands
  serialCommands.ReadSerial();
  // autoSleep1.checkSleep();
}
