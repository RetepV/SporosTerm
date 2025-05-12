#include <Preferences.h>

#pragma once

class BluetoothPreferences;
extern BluetoothPreferences bluetoothPreferences;

class BluetoothPreferences {
  
public:
  Preferences preferences;

  const char *preferencesName = "SporosTerm-BT";

  const char *prefVersionKey            = "VS";
  const int  preferencesVersion         = 2;

  const char *sendDelayEnabledKey       = "SDe";
  const char *sendDelayCharMillisecondsKey  = "SC";
  const char *sendDelayLineMillisecondsKey  = "SL";

  bool currentSendDelayEnabled = false;
  bool selectedSendDelayEnabled = false;

  // Amount of time to wait after sending a character. This is for slow systems, preventing overrun of their
  // hardware or software buffers. These defaults are empirically derived with a SYM-1 running at 4800 baud.
  int currentSendDelayCharMilliseconds = 60;
  int selectedSendDelayCharMilliseconds = 60;

  // Amount of time to wait after sending a CR. This is for slow systems, preventing overrun of their
  // hardware or software buffers. These defaults are empirically derived with a SYM-1 running at 4800 baud.
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

    currentSendDelayEnabled = selectedSendDelayEnabled;
    currentSendDelayCharMilliseconds = selectedSendDelayCharMilliseconds;
    currentSendDelayLineMilliseconds = selectedSendDelayLineMilliseconds;
  }

  bool isSendDelayEnabled() {
    return currentSendDelayEnabled;
  }

  int sendDelayForChar() {
    return currentSendDelayCharMilliseconds;
  }

  int sendDelayForLine() {
    return currentSendDelayLineMilliseconds;
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

BluetoothPreferences bluetoothPreferences;
