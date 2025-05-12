#include <Preferences.h>

#pragma once

class SerialPortPreferences;
extern SerialPortPreferences serialPortPreferences;

#define NUM_BAUDRATES     22
#define NUM_PARITIES      3
#define NUM_DATASIZES     4
#define NUM_STOPBITS      4
#define NUM_FLOWCONTROLS  4

#define UART_RX       34
#define UART_TX       2
#define UART_RTS      13
#define UART_CTS      35

#define DEFAULT_UART_INDEX    2
#define DEFAULT_UART_BAUD     9600
#define DEFAULT_UART_BITS     8
#define DEFAULT_UART_PARITY   'N'
#define DEFAULT_UART_STOPBITS 1.0
#define DEFAULT_UART_FLOW     fabgl::FlowControl::Hardware
#define DEFAULT_UART_INVERTED false

class SerialPortPreferences {
  
public:
  Preferences preferences;

  const char *preferencesName = "SporosTerm-SER";

  const char *prefVersionKey            = "VS";
  const int  preferencesVersion         = 2;

  const char *prefBaudRateKey           = "BR";
  const char *prefParityKey             = "PY";
  const char *prefDataSizeKey           = "DS";
  const char *prefStopBitsKey           = "SB";
  const char *prefFlowControlKey        = "FC";
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

  FlowControl selectableFlowControls[NUM_FLOWCONTROLS] = { FlowControl::None, FlowControl::Software, FlowControl::Hardware, FlowControl::Hardsoft };
  char *selectableFlowControlsStrings[NUM_FLOWCONTROLS] = { "None", "Xon/Xoff", "RTS/CTS or DSR/DTR", "Xon/Xoff with RTS/CTS or DSR/DTR" };
  int currentFlowControlIndex;
  int selectedFlowControlIndex;

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
    currentFlowControlIndex = determineFlowControlIndex((FlowControl)preferences.getInt(prefFlowControlKey));
    selectedFlowControlIndex = currentFlowControlIndex;
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

  void apply() {

    serialPort.setSignals(UART_RX, UART_TX, UART_RTS, UART_CTS, UART_RTS, UART_CTS, -1, -1);
    // delay(100);
    float stopBits = currentStopBits();
    Serial.printf("serialPort.setup: %6d,%c,%1d,%1d.%1d %d\n",
                                                 currentBaudRate(),
                                                 currentParity(),
                                                 currentDataSize(),
                                                 int(stopBits), int(10 * (stopBits - int(stopBits))),
                                                 currentFlowControl());

    serialPort.setup(DEFAULT_UART_INDEX, 
                     currentBaudRate(),
                     currentDataSize(),
                     currentParity(),
                     currentStopBits(),
                     currentFlowControl(),
                     DEFAULT_UART_INVERTED);

    Serial.printf("Serial port successfully setup\n");

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

  FlowControl currentFlowControl() {
    return selectableFlowControls[currentFlowControlIndex];
  }

  FlowControl selectedFlowControl() {
    return selectableFlowControls[selectedFlowControlIndex];
  }

  char *selectedFlowControlString() {
    return selectableFlowControlsStrings[selectedFlowControlIndex];
  }

  int determineFlowControlIndex(FlowControl flowControl) {
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
