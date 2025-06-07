#include "RelayManager.h"
#include <Preferences.h>

#pragma once

class SerialPortPreferences;
extern SerialPortPreferences serialPortPreferences;

#define NUM_BAUDRATES       22
#define NUM_PARITIES        3
#define NUM_DATASIZES       4
#define NUM_STOPBITS        4
#define NUM_FLOWCONTROLS    6
#define NUM_MODEMTYPES      2

#define UART_RX       34
#define UART_TX       2
#define UART_RTS      13
#define UART_CTS      35

#define DEFAULT_UART_INDEX    2
#define DEFAULT_UART_BAUD     2400
#define DEFAULT_UART_BITS     8
#define DEFAULT_UART_PARITY   'N'
#define DEFAULT_UART_STOPBITS 1.0
#define DEFAULT_UART_FLOW     SerialPortFlowControlType::SerialPortFlowControlNone
#define DEFAULT_MODEM_TYPE    ModemType::ModemTypeStraight
#define DEFAULT_UART_INVERTED false

enum SerialPortFlowControlType {
  SerialPortFlowControlNone,
  SerialPortFlowControlXON,
  SerialPortFlowControlRTS,
  SerialPortFlowControlDTR,
  SerialPortFlowControlXONRTS,
  SerialPortFlowControlXONDTR
};

enum ModemType {
  ModemTypeStraight = 0,
  ModemTypeNullModem
};

class SerialPortPreferences {
  
public:
  Preferences preferences;

  const char *preferencesName = "SporosTerm-SER";

  const char *prefVersionKey            = "VS";
  const int  preferencesVersion         = 3;

  const char *prefBaudRateKey           = "BR";
  const char *prefParityKey             = "PY";
  const char *prefDataSizeKey           = "DS";
  const char *prefStopBitsKey           = "SB";
  const char *prefFlowControlKey        = "FC";
  const char *prefModemTypeKey          = "MT";
  const char *sendDelayEnabledKey       = "SDe";
  const char *sendDelayCharMillisecondsKey  = "SC";
  const char *sendDelayLineMillisecondsKey  = "SL";

  int selectableBaudRates[NUM_BAUDRATES] = { 50, 60, 75, 110, 134, 150, 200, 300, 600, 1200, 1800, 2400, 4800, 9600, 19200, 31250, 38400, 57600, 115200, 230400, 460800, 921600 };
  char *selectableBaudRatesStrings[NUM_BAUDRATES] = { "50", "60", "75", "110", "134", "150", "200", "300", "600", "1200", "1800", "2400", "4800", "9600", "19200", "31250", "38400", "57600", "115200", "230400", "460800", "921600" };
  int currentBaudRateIndex;
  int selectedBaudRateIndex;

  char selectableParities[NUM_PARITIES] = { 'N', 'E', 'O' };
  char *selectableParitiesStrings[NUM_PARITIES] = { "None", "Even", "Odd" };
  int currentParityIndex;
  int selectedParityIndex;

  int selectableDataSizes[NUM_DATASIZES] = { 5, 6, 7, 8 };
  char *selectableDataSizesStrings[NUM_DATASIZES] = { "5", "6", "7", "8" };
  int currentDataSizeIndex;
  int selectedDataSizeIndex;

  float selectableStopBits[NUM_STOPBITS] = { 1, 1.5, 2, 3 };
  char *selectableStopBitsStrings[NUM_STOPBITS] = { "1", "1.5", "2", "3" };
  int currentStopBitsIndex;
  int selectedStopBitsIndex;

  SerialPortFlowControlType selectableFlowControls[NUM_FLOWCONTROLS] = {
    SerialPortFlowControlNone,
    SerialPortFlowControlXON,
    SerialPortFlowControlRTS,
    SerialPortFlowControlDTR,
    SerialPortFlowControlXONRTS,
    SerialPortFlowControlXONDTR
  };
  char *selectableFlowControlsStrings[NUM_FLOWCONTROLS] = {
    "None",
    "Xon/Xoff",
    "RTS/CTS",
    "DTR/DSR",
    "Xon/Xoff with RTS/CTS",
    "Xon/Xoff with DTR/DSR"
  };
  char *flowControlShortStrings[NUM_FLOWCONTROLS] = {
    "NON",
    "XON",
    "RTS",
    "DTR",
    "X&R",
    "X&D"
  };
  int currentFlowControlIndex;
  int selectedFlowControlIndex;

  ModemType selectableModemTypes[NUM_MODEMTYPES] = { ModemType::ModemTypeStraight, ModemType::ModemTypeNullModem};
  char *selectableModemTypesStrings[NUM_MODEMTYPES] = { "Straight through", "Null modem" };
  char *modemTypeShortStrings[NUM_MODEMTYPES] = { "STR", "NUL" };
  int currentModemTypeIndex;
  int selectedModemTypeIndex;

  bool currentSendDelayEnabled = false;
  bool selectedSendDelayEnabled = false;

  // Amount of time to wait after sending a character.
  int currentSendDelayCharMilliseconds = 50;
  int selectedSendDelayCharMilliseconds = 50;

  // Amount of time to wait 
  int currentSendDelayLineMilliseconds = 500;
  int selectedSendDelayLineMilliseconds = 500;

  bool needsReset = false;

  void start() {
    preferences.begin(preferencesName, true);
    // If there is no version key, we should create all settings with defaults.
    if (!preferences.isKey(prefVersionKey)) {
      convertOrReset(true);
    }
    // If there is a version key, but it differs from our current, we should create only new settings with defaults.
    int version = preferences.getInt(prefVersionKey);
    if (version != preferencesVersion) {
      convertOrReset(false);
    }

    fetch();
  }

  void end() {
    preferences.end();
  }

  void fetch() {
    currentBaudRateIndex = determineBaudRateIndex(preferences.getInt(prefBaudRateKey));
    selectedBaudRateIndex = currentBaudRateIndex;
    currentParityIndex = determineParityIndex(preferences.getChar(prefParityKey));
    selectedParityIndex = currentParityIndex;
    currentDataSizeIndex = determineDataSizeIndex(preferences.getInt(prefDataSizeKey));
    selectedDataSizeIndex = currentDataSizeIndex;
    currentStopBitsIndex = determineStopBitsIndex(preferences.getFloat(prefStopBitsKey));
    selectedStopBitsIndex = currentStopBitsIndex;
    currentFlowControlIndex = determineFlowControlIndex((SerialPortFlowControlType)preferences.getInt(prefFlowControlKey));
    selectedFlowControlIndex = currentFlowControlIndex;
    currentModemTypeIndex = determineModemTypeIndex((ModemType)preferences.getInt(prefModemTypeKey));
    selectedModemTypeIndex = currentModemTypeIndex;
    currentSendDelayEnabled = preferences.getBool(sendDelayEnabledKey);
    selectedSendDelayEnabled = currentSendDelayEnabled;
    currentSendDelayCharMilliseconds = preferences.getInt(sendDelayCharMillisecondsKey);
    selectedSendDelayCharMilliseconds = currentSendDelayCharMilliseconds;
    currentSendDelayLineMilliseconds = preferences.getInt(sendDelayLineMillisecondsKey);
    selectedSendDelayLineMilliseconds = currentSendDelayLineMilliseconds;
  }

  void save() {

    preferences.end();
    preferences.begin(preferencesName, false);

    preferences.putInt(prefBaudRateKey, selectedBaudRate());
    preferences.putChar(prefParityKey, selectedParity());
    preferences.putInt(prefDataSizeKey, selectedDataSize());
    preferences.putFloat(prefStopBitsKey, selectedStopBits());
    preferences.putInt(prefFlowControlKey, (int)selectedFlowControl());
    preferences.putInt(prefModemTypeKey, (int)selectedModemType());
    preferences.putBool(sendDelayEnabledKey, selectedSendDelayEnabled);
    preferences.putInt(sendDelayCharMillisecondsKey, selectedSendDelayCharMilliseconds);
    preferences.putInt(sendDelayLineMillisecondsKey, selectedSendDelayLineMilliseconds);

    preferences.putInt(prefVersionKey, preferencesVersion);

    preferences.end();
    preferences.begin(preferencesName, true);

    fetch();
  }

  void convertOrReset(bool reset) {

    preferences.end();
    preferences.begin(preferencesName, false);

    if (reset) {
      preferences.clear();
    }

    if (!preferences.isKey(prefBaudRateKey)) {
      preferences.putInt(prefBaudRateKey, DEFAULT_UART_BAUD);
    }
    if (!preferences.isKey(prefParityKey)) {
      preferences.putChar(prefParityKey, DEFAULT_UART_PARITY);
    }
    if (!preferences.isKey(prefDataSizeKey)) {
      preferences.putInt(prefDataSizeKey, DEFAULT_UART_BITS);
    }
    if (!preferences.isKey(prefStopBitsKey)) {
      preferences.putFloat(prefStopBitsKey, DEFAULT_UART_STOPBITS);
    }
    if (!preferences.isKey(prefFlowControlKey)) {
      preferences.putInt(prefFlowControlKey, (int)DEFAULT_UART_FLOW);
    }
    if (!preferences.isKey(prefModemTypeKey)) {
      preferences.putInt(prefModemTypeKey, (int)DEFAULT_MODEM_TYPE);
    }
    if (!preferences.isKey(sendDelayEnabledKey)) {
      preferences.putBool(sendDelayEnabledKey, selectedSendDelayEnabled);
    }
    if (!preferences.isKey(sendDelayCharMillisecondsKey)) {
      preferences.putInt(sendDelayCharMillisecondsKey, selectedSendDelayCharMilliseconds);
    }
    if (!preferences.isKey(sendDelayLineMillisecondsKey)) {
      preferences.putInt(sendDelayLineMillisecondsKey, selectedSendDelayLineMilliseconds);
    }

    preferences.putInt(prefVersionKey, preferencesVersion);

    preferences.end();
    preferences.begin(preferencesName, true);

    fetch();
  }

  fabgl::FlowControl fabglFlowControl(SerialPortFlowControlType flowControl) {

    switch (flowControl) {
      case SerialPortFlowControlNone:
        return fabgl::FlowControl::None;
      case SerialPortFlowControlXON:
        return fabgl::FlowControl::Software;
      case SerialPortFlowControlRTS:
      case SerialPortFlowControlDTR:
        return fabgl::FlowControl::Hardware;
      case SerialPortFlowControlXONRTS:
      case SerialPortFlowControlXONDTR:
        return fabgl::FlowControl::Hardsoft;
      default:
        return fabgl::FlowControl::None;
    }
  }

  bool isDtrDsr(SerialPortFlowControlType flowControl) {
    switch (flowControl) {
      case SerialPortFlowControlDTR:
      case SerialPortFlowControlXONDTR:
        return true;
      default:
        return false;
    }
  }

  bool isStraightModem(ModemType modemType) {
    return modemType == ModemTypeStraight;
  }

  void apply() {

    serialPort.setSignals(UART_RX, UART_TX, UART_RTS, UART_CTS, UART_RTS, UART_CTS, -1, -1);

    float stopBits = currentStopBits();

    serialPort.setup(DEFAULT_UART_INDEX, 
                     currentBaudRate(),
                     currentDataSize(),
                     currentParity(),
                     currentStopBits(),
                     fabglFlowControl(currentFlowControl()),
                     DEFAULT_UART_INVERTED);

    relayManager.setRelays(!isStraightModem(currentModemType()),isDtrDsr(currentFlowControl()));

    currentSendDelayEnabled = selectedSendDelayEnabled;
    currentSendDelayCharMilliseconds = selectedSendDelayCharMilliseconds;
    currentSendDelayLineMilliseconds = selectedSendDelayLineMilliseconds;
  }

  int currentBaudRate() {
    return selectableBaudRates[currentBaudRateIndex];
  }

  int selectedBaudRate() {
    return selectableBaudRates[selectedBaudRateIndex];
  }

  char *selectedBaudRateString() {
    return selectableBaudRatesStrings[selectedBaudRateIndex];
  }

  int determineBaudRateIndex(int baudRate) {
    for (int index = 0; index < NUM_BAUDRATES; index++) {
      if (selectableBaudRates[index] == baudRate) {
        return index;
      }
    }
    return 0;
  }

  void selectNextBaudRate() {
    selectedBaudRateIndex++;
    if (selectedBaudRateIndex >= NUM_BAUDRATES) {
      selectedBaudRateIndex = 0;
    }
  }

  void selectPrevBaudRate() {
    if (selectedBaudRateIndex == 0) {
      selectedBaudRateIndex = NUM_BAUDRATES - 1;
    }
    else {
      selectedBaudRateIndex--;
    }
  }

  char currentParity() {
    return selectableParities[currentParityIndex];
  }

  char selectedParity() {
    return selectableParities[selectedParityIndex];
  }

  char *selectedParityString() {
    return selectableParitiesStrings[selectedParityIndex];
  }

  int determineParityIndex(char parity) {
    for (int index = 0; index < NUM_PARITIES; index++) {
      if (selectableParities[index] == parity) {
        return index;
      }
    }
    return 0;
  }

  void selectNextParity() {
    selectedParityIndex++;
    if (selectedParityIndex >= NUM_PARITIES) {
      selectedParityIndex = 0;
    }
  }

  void selectPrevParity() {
    if (selectedParityIndex == 0) {
      selectedParityIndex = NUM_PARITIES - 1;
   }
   else {
      selectedParityIndex--;
    }
  }

  int currentDataSize() {
    return selectableDataSizes[currentDataSizeIndex];
  }

  int selectedDataSize() {
    return selectableDataSizes[selectedDataSizeIndex];
  }

  char *selectedDataSizeString() {
    return selectableDataSizesStrings[selectedDataSizeIndex];
  }

  int determineDataSizeIndex(int dataSize) {
    for (int index = 0; index < NUM_DATASIZES; index++) {
      if (selectableDataSizes[index] == dataSize) {
        return index;
      }
    }
    return 0;
  }

  void selectNextDataSize() {
    selectedDataSizeIndex++;
    if (selectedDataSizeIndex >= NUM_DATASIZES) {
      selectedDataSizeIndex = 0;
    }
  }

  void selectPrevDataSize() {
    if (selectedDataSizeIndex == 0) {
      selectedDataSizeIndex = NUM_DATASIZES - 1;
   }
   else {
      selectedDataSizeIndex--;
    }
  }

  float currentStopBits() {
    return selectableStopBits[currentStopBitsIndex];
  }

  float selectedStopBits() {
    return selectableStopBits[selectedStopBitsIndex];
  }

  char *selectedStopBitsString() {
    return selectableStopBitsStrings[selectedStopBitsIndex];
  }

  int determineStopBitsIndex(float stopBits) {
    for (int index = 0; index < NUM_STOPBITS; index++) {
      if ((stopBits - 0.1) < selectableStopBits[index] < (stopBits + 0.1)) {
        return index;
      }
    }
    return 0;
  }

  void selectNextStopBits() {
    selectedStopBitsIndex++;
    if (selectedStopBitsIndex >= NUM_STOPBITS) {
      selectedStopBitsIndex = 0;
    }
  }

  void selectPrevStopBits() {
    if (selectedStopBitsIndex == 0) {
      selectedStopBitsIndex = NUM_STOPBITS - 1;
   }
   else {
      selectedStopBitsIndex--;
    }
  }

  SerialPortFlowControlType currentFlowControl() {
    return selectableFlowControls[currentFlowControlIndex];
  }

  SerialPortFlowControlType selectedFlowControl() {
    return selectableFlowControls[selectedFlowControlIndex];
  }

  char *selectedFlowControlString() {
    return selectableFlowControlsStrings[selectedFlowControlIndex];
  }

  char *currentFlowControlShortString() {
    return flowControlShortStrings[currentFlowControlIndex];
  }

  int determineFlowControlIndex(SerialPortFlowControlType flowControl) {
    for (int index = 0; index < NUM_FLOWCONTROLS; index++) {
      if (selectableFlowControls[index] == flowControl) {
        return index;
      }
    }
    return 0;
  }

  void selectNextFlowControl() {
    selectedFlowControlIndex++;
    if (selectedFlowControlIndex >= NUM_FLOWCONTROLS) {
      selectedFlowControlIndex = 0;
    }
  }

  void selectPrevFlowControl() {
    if (selectedFlowControlIndex == 0) {
      selectedFlowControlIndex = NUM_FLOWCONTROLS - 1;
   }
   else {
      selectedFlowControlIndex--;
    }
  }

  ModemType currentModemType() {
    return selectableModemTypes[currentModemTypeIndex];
  }

  ModemType selectedModemType() {
    return selectableModemTypes[selectedModemTypeIndex];
  }

  char *currentModemTypeShortString() {
    return modemTypeShortStrings[currentModemTypeIndex];
  }

  char *selectedModemTypeString() {
    return selectableModemTypesStrings[selectedModemTypeIndex];
  }

  int determineModemTypeIndex(ModemType modemType) {
    for (int index = 0; index < NUM_MODEMTYPES; index++) {
      if (selectableModemTypes[index] == modemType) {
        return index;
      }
    }
    return 0;
  }

  void selectNextModemType() {
    selectedModemTypeIndex++;
    if (selectedModemTypeIndex >= NUM_MODEMTYPES) {
      selectedModemTypeIndex = 0;
    }
  }

  void selectPrevModemType() {
    if (selectedModemTypeIndex == 0) {
      selectedModemTypeIndex = NUM_MODEMTYPES - 1;
   }
   else {
      selectedModemTypeIndex--;
    }
  }

  void toggleSendDelayEnabled() {
    selectedSendDelayEnabled = !selectedSendDelayEnabled;
  }

  void incSendDelayChar(int inc) {
    selectedSendDelayCharMilliseconds += inc;
    // Let's cap at 10 seconds max per character.
    if (selectedSendDelayCharMilliseconds > 10000) {
      selectedSendDelayCharMilliseconds = 10000;
    }
  }

  void decSendDelayChar(int dec) {
    selectedSendDelayCharMilliseconds -= dec;
    if (selectedSendDelayCharMilliseconds < 0) {
      selectedSendDelayCharMilliseconds = 0;
    }
  }

  void incSendDelayLine(int inc) {
    selectedSendDelayLineMilliseconds += inc;
    // Let's cap at 10 seconds max per character.
    if (selectedSendDelayLineMilliseconds > 10000) {
      selectedSendDelayLineMilliseconds = 10000;
    }
  }

  void decSendDelayLine(int dec) {
    selectedSendDelayLineMilliseconds -= dec;
    if (selectedSendDelayLineMilliseconds < 0) {
      selectedSendDelayLineMilliseconds = 0;
    }
  }
};

SerialPortPreferences serialPortPreferences;
