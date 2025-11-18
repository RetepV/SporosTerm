
#pragma once

enum SettingsPageAction {
  noFurtherAction,
  endSettings,
  gotoMainSettingsPage,
  gotoSerialPortSettingsPage,
  gotoTerminalSettingsPage,
  gotoDisplaySettingsPage,
  gotoBluetoothSettingsPage,
  gotoCapabilitiesPage,
  gotoDateTimeSettingsPage
};

class SettingsManagerPage {
public:
  virtual void show();
  virtual SettingsPageAction handleChoice(uint8_t choice);
};

