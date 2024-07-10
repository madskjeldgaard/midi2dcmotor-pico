/*
 *
 * This file represents a DRV8833 motor driver chip.  The DRV8833 is a dual
 * H-bridge motor driver that can drive two DC motors or one stepper motor.
 *
 * See the datasheet here:
 * https://www.ti.com/lit/ds/symlink/drv8833.pdf
 *
 *
 */

#pragma once
#include <Arduino.h>

namespace motor {

enum class DecayMode {
  Slow = 0,
  Fast = 1,
};

enum class Direction {
  Forward,
  Backward,
};

/**
 * @class DRV8833_HBridge
 * @brief One bridge of a DRV8833 motor driver chip. This repsentation of a
 * bridge supports fast and slow decay modes.
 *
 */
class DRV8833_HBridge {
public:
  // Constructor
  // in1 and in2 are the pins connected to the motor
  // The default decay mode is slow
  /**
   * @brief Default constructor for the DRV8833_HBridge class. The default
   * decay mode is slow.
   *
   * @param in1 pin1 of the motor driver bridge
   * @param in2 pin2 of the motor driver bridge
   */
  DRV8833_HBridge(uint8_t in1, uint8_t in2) : mIn1(in1), mIn2(in2) {
    pinMode(in1, OUTPUT);
    pinMode(in2, OUTPUT);
    mDecayMode = DecayMode::Slow;
  }

  /**
   * @brief Constructor for the DRV8833_HBridge class.  The decay mode can be
   * specified.
   *
   * @param in1 pin1 of the motor driver bridge
   * @param in2 pin2 of the motor driver bridge
   * @param mode Decaymode of the motor driver bridge
   */
  DRV8833_HBridge(uint8_t in1, uint8_t in2, DecayMode mode)
      : mIn1(in1), mIn2(in2), mDecayMode(mode) {
    pinMode(in1, OUTPUT);
    pinMode(in2, OUTPUT);
  }

  void setDecayMode(DecayMode mode) { mDecayMode = mode; }

  /**
   * @brief Set speed. This version of the method assumes the motor is moving in
   * the forward direction.
   *
   * @param speed The speed of the motor.  The speed is a value between 0 and
   * 1023.
   */
  void setSpeed(uint16_t speed) { setSpeed(speed, Direction::Forward); }

  /**
   * @brief Set speed and direction of the motor
   *
   * @param speed The speed of the motor.  The speed is a value between 0 and
   * 1023.
   * @param dir The direction of the motor.  Forward or Backwards.
   */
  void setSpeed(uint16_t speed, Direction dir) {

    switch (mDecayMode) {
    case DecayMode::Fast:

      switch (dir) {
      case Direction::Forward:
        digitalWrite(mIn1, speed);
        analogWrite(mIn2, LOW);
        break;
      case Direction::Backward:
        digitalWrite(mIn1, LOW);
        analogWrite(mIn2, speed);
        break;
      }
      break;
    case DecayMode::Slow:

      switch (dir) {
      case Direction::Forward:

        digitalWrite(mIn1, HIGH);
        analogWrite(mIn2, speed);
        break;
      case Direction::Backward:
        digitalWrite(mIn1, speed);
        analogWrite(mIn2, HIGH);
        break;
      }

      break;
    }
  }

  /**
   * @brief Set the speed and direction of the motor
   *
   * @param speed The speed of the motor, between -1023 and 1023. Positive
   * values are forward, negative values are backward, and 0 is stopped.
   *
   */
  void setSpeedBipolar(int16_t speed) {
    if (speed > 0) {
      setSpeed(speed, Direction::Forward);
    } else if (speed < 0) {
      setSpeed(speed, Direction::Backward);
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

  DecayMode mDecayMode;
};

/**
 * @class DRV8833
 * @brief Represents a DRV8833 motor driver with two H-bridges and a sleep pin.
 * Each H Bridge may be configured in a fast or slow decay mode.
 *
 * For more information about decay modes and their impact on performance, see
 * this article:
 * https://learn.adafruit.com/improve-brushed-dc-motor-performance/overview
 *
 */
class DRV8833 {
public:
  /**
   * @brief Default constructor for the DRV8833 class.  The default decay mode
   * is slow.
   *
   * @param in1 Pin connected to AIN1 on the DRV8833
   * @param in2 Pin connected to AIN2 on the DRV8833
   * @param in3 Pin connected to BIN1 on the DRV8833
   * @param in4 Pin connected to BIN2 on the DRV8833
   * @param sleep Pin connected to SLEEP on the DRV8833
   */
  DRV8833(uint8_t in1, uint8_t in2, uint8_t in3, uint8_t in4, uint8_t sleep)
      : mBridgeA(in1, in2), mBridgeB(in3, in4), mSleep(sleep) {
    pinMode(sleep, OUTPUT);
    digitalWrite(sleep, HIGH);
  }

  /**
   * @brief Constructor for the DRV8833 class. The decay mode can be specified.
   *
   * @param in1 Pin connected to AIN1 on the DRV8833
   * @param in2 Pin connected to AIN2 on the DRV8833
   * @param in3 Pin connected to BIN1 on the DRV8833
   * @param in4 Pin connected to BIN2 on the DRV8833
   * @param sleep Pin connected to SLEEP on the DRV8833
   * @param mode Decay mode of the motor driver chip
   */
  DRV8833(uint8_t in1, uint8_t in2, uint8_t in3, uint8_t in4, uint8_t sleep,
          DecayMode mode)
      : mBridgeA(in1, in2, mode), mBridgeB(in3, in4, mode), mSleep(sleep) {
    pinMode(sleep, OUTPUT);
    digitalWrite(sleep, HIGH);
  }

  void stopAll() {
    mBridgeA.stop();
    mBridgeB.stop();
  }

  auto &getBridgeA() { return mBridgeA; }
  auto &getBridgeB() { return mBridgeB; }

  void sleep() { digitalWrite(mSleep, LOW); }
  void wake() { digitalWrite(mSleep, HIGH); }

private:
  DRV8833_HBridge mBridgeA, mBridgeB;
  uint8_t mSleep;
};

} // namespace motor
