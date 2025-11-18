#include <Preferences.h>

#pragma once

class DisplayPreferences;
extern DisplayPreferences displayPreferences;

#define NUM_DISPLAYMODES      14
#define DEFAULT_DISPLAYMODE   0

#define SVGA_800x480_66Hz "\"800x480@66Hz\" 36 800 824 896 1024 480 490 492 525 -HSync -VSync"
#define SVGA_800x480_66Hz_Alt "\"800x480@66Hz\" 36 800 824 896 1024 480 494 496 533 -HSync -VSync"
#define SVGA_800x480_60Hz "\"800x480@60Hz\" 40 800 840 968 1056 480 490 492 525 -HSync -VSync"

#define SVGA_672_352_70Hz "\"672x352@70Hz\" 25.175 672 688 752 800 352 387 389 430 -HSync -VSync"

#define VGA_800_500_56Hz "\"800x500@56Hz\" 36 800 824 896 1024 500 561 575 625 -HSync -VSync"

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

  // Modes with Bluetooth enabled. Monochrome.  

  {
    "640x350, 80x24, Monochrome, Status Bar, Bluetooth",
    VGA_640x350_70Hz,
    &fabgl::FONT_SPOROSTERM_8X14,
    2,
    80,
    24,
    640,
    350,
    true,
    true
  },
  {
    "640x350, 80x25, Monochrome, Bluetooth",
    VGA_640x350_70Hz,
    &fabgl::FONT_SPOROSTERM_8X14,
    2,
    80,
    25,
    640,
    350,
    false,
    true
  },
  {
    "640x350, 80x48, Monochrome, Status Bar, Bluetooth",
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
    "640x350, 80x50, Monochrome, Bluetooth",
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
    "672x352, 132x48, Monochrome, Status Bar, Bluetooth",
    SVGA_672_352_70Hz,
    &fabgl::FONT_SPOROSTERM_5X7,
    2,
    132,
    48,
    672,
    352,
    true,
    true
  },
  {
    "672x352, 132x50, Monochrome, Bluetooth",
    SVGA_672_352_70Hz,
    &fabgl::FONT_SPOROSTERM_5X7,
    2,
    132,
    50,
    672,
    352,
    false,
    true
  },

  // 16-color modes, no bluetooth

  {
    "640x350, 80x24, 16 Color, Status Bar",
    VGA_640x350_70Hz,
    &fabgl::FONT_SPOROSTERM_8X14,
    16,
    80,
    24,
    640,
    350,
    true,
    false
  },
  {
    "640x350, 80x24, 16 Color",
    VGA_640x350_70Hz,
    &fabgl::FONT_SPOROSTERM_8X14,
    16,
    80,
    25,
    640,
    350,
    false,
    false
  },
  {
    "640x350, 80x48, 16 Color, Status Bar",
    VGA_640x350_70Hz,
    &fabgl::FONT_8x7,
    16,
    80,
    48,
    640,
    350,
    true,
    false
  },
  {
    "640x350, 80x50, 16 Color",
    VGA_640x350_70Hz,
    &fabgl::FONT_8x7,
    16,
    80,
    50,
    640,
    350,
    false,
    false
  },
  {
    "672x352, 132x48, 16 Color, Status Bar",
    SVGA_672_352_70Hz,
    &fabgl::FONT_SPOROSTERM_5X7,
    16,
    132,
    48,
    672,
    352,
    false,
    true
  },
  {
    "672x352, 132x50, 16 Color",
    SVGA_672_352_70Hz,
    &fabgl::FONT_SPOROSTERM_5X7,
    16,
    132,
    50,
    672,
    352,
    false,
    true
  },

  // DEC 800x500 compatible modes, 8 colors unfortunately, due to not enough memory.

  {
    "DEC 800x500 80x24 8 color, with status bar",
    VGA_800_500_56Hz,
    &fabgl::FONT_SPOROSTERM_10X20,
    8,
    80,
    24,
    800,
    500,
    true,
    false
  },
  {
    "DEC 800x500 80x25 8 color",
    VGA_800_500_56Hz,
    &fabgl::FONT_SPOROSTERM_10X20,
    8,
    80,
    25,
    800,
    500,
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
