#include <Preferences.h>

#pragma once

class DisplayPreferences;
extern DisplayPreferences displayPreferences;

#define NUM_DISPLAYMODES      8

#define DEFAULT_DISPLAYMODE   0

#define SVGA_800x480_66Hz "\"800x480@66Hz\" 36 800 824 896 1024 480 490 492 525 -HSync -VSync"
#define SVGA_800x480_66Hz_Alt "\"800x480@66Hz\" 36 800 824 896 1024 480 494 496 533 -HSync -VSync"
#define SVGA_800x480_60Hz "\"800x480@60Hz\" 40 800 840 968 1056 480 490 492 525 -HSync -VSync"

// SVGA_800x600_60Hz "\"800x600@60Hz\" 40 800 840 968 1056
// 480 490 492 525 -HSync -VSync

struct DisplayMode {
  char const *resolution;
  char const *modeString;
  fabgl::FontInfo const *font;
  int colors;
  int columns;
  int rows;
  int xRes;
  int yRes;
  bool enableStatusBar;
  bool supportsBluetooth;
};

struct DisplayMode displayModes[NUM_DISPLAYMODES] = {
  {
    "640x350 70Hz",
    VGA_640x350_70Hz,
    &fabgl::FONT_8x14,
    2,
    80,
    24,
    640,
    350,
    true,
    true
  },
  {
    "640x350 70Hz",
    VGA_640x350_70Hz,
    &fabgl::FONT_8x14,
    2,
    80,
    25,
    640,
    350,
    false,
    true
  },
  {
    "640x350 70Hz",
    VGA_640x350_70Hz,
    &fabgl::FONT_8x7,
    2,
    80,
    48,
    640,
    350,
    true,
    true
  },
  {
    "640x350 70Hz",
    VGA_640x350_70Hz,
    &fabgl::FONT_8x7,
    2,
    80,
    50,
    640,
    350,
    false,
    true
  },

  {
    "800x480 66Hz",
    SVGA_800x480_66Hz_Alt,
    &fabgl::FONT_10x19_DEC,
    8,
    80,
    24,
    800,
    480,
    true,
    false
  },
  {
    "800x480 66Hz",
    SVGA_800x480_60Hz,
    &fabgl::FONT_10x19_DEC,
    8,
    80,
    25,
    800,
    480,
    false,
    false
  },
  {
    "800x480 66Hz",
    SVGA_800x480_60Hz,
    &fabgl::FONT_8x9,
    8,
    80,
    48,
    800,
    480,
    true,
    false
  },
  {
    "800x480 66Hz",
    SVGA_800x480_60Hz,
    &fabgl::FONT_8x9,
    8,
    80,
    50,
    800,
    480,
    false,
    false
  },
};

class DisplayPreferences {
  
public:
  Preferences preferences;

  const char *preferencesName = "SporosTerm-DSP";

  const char *prefVersionKey                  = "VS";
  const int  preferencesVersion               = 1;

  const char *prefDisplayModeIndexKey         = "DM";

  int currentDisplayModeIndex;
  int selectedDisplayModeIndex;

  bool needsRestart = false;

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
    currentDisplayModeIndex = preferences.getInt(prefDisplayModeIndexKey);
    selectedDisplayModeIndex = currentDisplayModeIndex;
  }

  void save() {

    preferences.end();
    preferences.begin(preferencesName, false);

    preferences.putInt(prefDisplayModeIndexKey, selectedDisplayModeIndex);

    if (selectedDisplayModeIndex != currentDisplayModeIndex) {
      needsRestart = true;
    }
    else {
      needsRestart = false;
    }

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

    if (!preferences.isKey(prefDisplayModeIndexKey)) {
      preferences.putInt(prefDisplayModeIndexKey, DEFAULT_DISPLAYMODE);
    }

    preferences.putInt(prefVersionKey, preferencesVersion);

    preferences.end();
    preferences.begin(preferencesName, true);

    fetch();
  }

  bool hasChanges() {
    return (selectedDisplayModeIndex != currentDisplayModeIndex);
  }

  void apply() {

    // In this case, we need to restart. The new settings will be picked up at restart.
    // NOTE: Probably we can do this without a restart.

    ESP.restart();
  }

  DisplayMode currentDisplayMode() {
    return displayModes[currentDisplayModeIndex];
  }

  DisplayMode selectedDisplayMode() {
    return displayModes[selectedDisplayModeIndex];
  }

  void selectNextDisplayMode() {
    selectedDisplayModeIndex++;
    if (selectedDisplayModeIndex >= NUM_DISPLAYMODES) {
      selectedDisplayModeIndex = 0;
    }
  }

  void selectPrevDisplayMode() {
    if (selectedDisplayModeIndex == 0) {
      selectedDisplayModeIndex = NUM_DISPLAYMODES - 1;
    }
    else {
      selectedDisplayModeIndex--;
    }
  }
};

DisplayPreferences displayPreferences;
