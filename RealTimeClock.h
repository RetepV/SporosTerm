#include <OneWire.h>
#include <functional>

#pragma once

class RealTimeClock;
extern RealTimeClock realTimeClock;

class RealTimeClock {

public:

  static const byte ds2417Family = 0x27;

  static const byte cmdReadClock = 0x66;
  static const byte cmdWriteClock = 0x99;
  static const byte cmdReadROM = 0x33;
  static const byte cmdMatchROM = 0x55;
  static const byte cmdSearchROM = 0xF0;
  static const byte cmdSkipROM = 0xCC;

  static const byte IEBitMask = 0x80;
  static const byte ISBitsMask = 0x70;
  static const byte OscBitsMask = 0x0C;

  int oneWireGPIOPin = -1;

  OneWire *oneWire = NULL;

  bool devicePresent = false;
  byte deviceAddress[8];
  byte clockFunctionData[5];

  void setup(int gpioPin) {

    oneWireGPIOPin = gpioPin;
    devicePresent = false;

    memset(deviceAddress, 0, 8);
    memset(clockFunctionData, 0, 8);

    oneWire = new OneWire(oneWireGPIOPin);
  }

  bool findDevice() {

    oneWire->reset_search();
    // Necessary delay?
    delay(250);

    while (true) {
      if (oneWire->search(deviceAddress)) {
        // Found a device, is it a DS2417 family device?
        if (deviceAddress[0] == ds2417Family) {
          if (OneWire::crc8(deviceAddress, 7) != deviceAddress[7]) {
            return false;
          }
          // Found device. We only support one, so we ignore any other.
          devicePresent = true;
          return true;
        }
      }
      else {
        // No (more) device(s), no DS2417 found.
        devicePresent = false;
        return false;
      }
    }
  }

  time_t readTime() {

    oneWire->reset();
    oneWire->select(deviceAddress);
    oneWire->write(cmdReadClock);
    // Expect 5 bytes
    for (int index = 0; index < 5; index++) {
      clockFunctionData[index] = oneWire->read();
    }

    return (time_t)( (unsigned int)clockFunctionData[1] + 
                    ((unsigned int)clockFunctionData[2] << 8) +
                    ((unsigned int)clockFunctionData[3] << 16) +
                    ((unsigned int)clockFunctionData[4] << 24));
  }

  void writeTime(time_t time) {

    // Don't touch clockFunctionData[0], use what was read the last time.
    clockFunctionData[1] = time & 0xff;
    clockFunctionData[2] = (time >> 8) & 0xFF;
    clockFunctionData[3] = (time >> 16) & 0xFF;
    clockFunctionData[4] = (time >> 24) & 0xFF;

    oneWire->reset();
    oneWire->select(deviceAddress);
    oneWire->write(cmdWriteClock);
    // Send 5 bytes
    for (int index = 0; index < 5; index++) {
      oneWire->write(clockFunctionData[index]);
    }
  }

  void updateStatus(std::function<byte(byte)>update) {

    oneWire->reset();
    oneWire->select(deviceAddress);
    oneWire->write(cmdReadClock);
    // Expect 5 bytes
    for (int index = 0; index < 5; index++) {
      clockFunctionData[index] = oneWire->read();
    }

    // clockFunctionData is the status register, leave the rest alone.
    clockFunctionData[0] = update(clockFunctionData[0]);

    oneWire->reset();
    oneWire->select(deviceAddress);
    oneWire->write(cmdWriteClock);
    // Send 5 bytes
    for (int index = 0; index < 5; index++) {
      oneWire->write(clockFunctionData[index]);
    }
  }

  bool isClockRunning() {
    readTime();
    return ((clockFunctionData[0] & OscBitsMask) == OscBitsMask);
  }

  bool startStopClock(bool start) {
    updateStatus([start](byte currentStatus) {
      return start ? currentStatus | OscBitsMask : currentStatus & ~OscBitsMask;
    });
  }
};

RealTimeClock realTimeClock;