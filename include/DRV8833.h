#pragma once
#include <Arduino.h>

namespace motor {

// A simple arduino class to control a DRV8833 motor driver
class DRV8833Channel {
public:
  DRV8833Channel(uint8_t in1, uint8_t in2) : mIn1(in1), mIn2(in2) {
    pinMode(in1, OUTPUT);
    pinMode(in2, OUTPUT);
  }

  // The resolution of the PWM speed is 10 bits, so the speed can be between
  // -1023 and 1023
  void setSpeed(int16_t speed) {
    if (speed > 0) {

      analogWrite(mIn1, abs(speed));
      digitalWrite(mIn2, LOW);

    } else if (speed < 0) {
      digitalWrite(mIn1, LOW);
      analogWrite(mIn2, abs(speed));

    } else {
      stop();
    }
  }

  void stop() {
    digitalWrite(mIn1, LOW);
    digitalWrite(mIn2, LOW);
  }

private:
  uint8_t mIn1, mIn2;
};

// Represents a two channel DRV8833 motor driver with sleep pin
class DRV8833
{
public:
  DRV8833(uint8_t in1, uint8_t in2, uint8_t in3, uint8_t in4, uint8_t sleep)
      : mChannelA(in1, in2), mChannelB(in3, in4), mSleep(sleep) {
    pinMode(sleep, OUTPUT);
    digitalWrite(sleep, HIGH);
  }

  void setSpeedA(int16_t speed) {
    mChannelA.setSpeed(speed);
  }

  void setSpeedB(int16_t speed) {
    mChannelB.setSpeed(speed);
  }

  void stopAll() {
    mChannelA.stop();
    mChannelB.stop();
  }

  void stopA () {
    mChannelA.stop();
  }

  void stopB () {
    mChannelB.stop();
  }

  void sleep() { digitalWrite(mSleep, LOW); }
  void wake() { digitalWrite(mSleep, HIGH); }

private:
  DRV8833Channel mChannelA, mChannelB;
  uint8_t mSleep;
};

} // namespace motor
