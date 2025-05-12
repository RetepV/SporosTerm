#include <Preferences.h>

#pragma once

class TerminalPreferences;
extern TerminalPreferences terminalPreferences;

#define NUM_CURSORSTYLES                  3

#define DEFAULT_NEWLINEMODE               true
#define DEFAULT_SMOOTH_SCROLL             true
#define DEFAULT_AUTOREPEAT                true
#define DEFAULT_CURSOR_BLINK              true
#define DEFAULT_CURSOR_STYLE              1
#define DEFAULT_BACKSPACE_STYLE           true
#define DEFAULT_NEWLINE_MODE              true
#define DEFAULT_WRAPAROUND                true
#define DEFAULT_REVERSE_WRAPAROUND        true

class TerminalPreferences {
  
public:
  Preferences preferences;

  const char *preferencesName = "SporosTerm-TRM";

  const char *prefVersionKey            = "VS";
  const int  preferencesVersion         = 1;

  const char *prefNewlineModeKey        = "NL";         // TRUE: CRLF, FALSE: CR only
  const char *prefSmoothScrollKey       = "SS";         // TRUE: smooth scroll, FALSE: jump scroll
  const char *prefKeyAutoRepeatKey      = "AR";         // TRUE: autorepeat, FALSE: mo autorepeat
  const char *prefCursorBlinkKey        = "CB";         // TRUE: cursor blink, FALSE: cursor doesn't blink
  const char *prefCursorStyleKey        = "CS";         // 0 .. 2: Block, 3..4: underline, 5..6: bar
  const char *prefBackspaceStyleKey     = "BS";         // TRUE: BS, FALSE: DEL
  const char *prefWrapAroundKey         = "WA";         // TRUE: text wrap around, FALSE: text does not wrap.
  const char *prefReverseWrapAroundKey  = "RW";         // TRUE: text reverse wrap around, FALSE: text does not reverse wrap

  bool currentNewLineMode;
  bool selectedNewLineMode;
  bool currentSmoothScroll;
  bool selectedSmoothScroll;
  bool currentAutoRepeat;
  bool selectedAutoRepeat;
  bool currentCursorBlink;
  bool selectedCursorBlink;
  int selectableCursorStyles[NUM_DATASIZES] = { 1, 3, 5 };
  char *selectableCursorStylesStrings[NUM_DATASIZES] = { "Block", "Underline", "Bar" };
  int currentCursorStyleIndex;
  int selectedCursorStyleIndex;
  bool currentBackspaceStyle;
  bool selectedBackspaceStyle;
  bool currentWrapAround;
  bool selectedWrapAround;
  bool currentReverseWrapAround;
  bool selectedReverseWrapAround;
  
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
    currentNewLineMode = preferences.getBool(prefNewlineModeKey);
    selectedNewLineMode = currentNewLineMode;
    currentSmoothScroll = preferences.getBool(prefSmoothScrollKey);
    selectedSmoothScroll = currentSmoothScroll;
    currentAutoRepeat = preferences.getBool(prefKeyAutoRepeatKey);
    selectedAutoRepeat = currentAutoRepeat;
    currentCursorBlink = preferences.getBool(prefCursorBlinkKey);
    selectedCursorBlink = currentCursorBlink;
    currentCursorStyleIndex = determineCursorStyleIndex(preferences.getInt(prefCursorStyleKey));
    selectedCursorStyleIndex = currentCursorStyleIndex;
    currentBackspaceStyle = preferences.getBool(prefBackspaceStyleKey);
    selectedBackspaceStyle = currentBackspaceStyle;
    currentWrapAround = preferences.getBool(prefWrapAroundKey);
    selectedWrapAround = currentWrapAround;
    currentReverseWrapAround = preferences.getBool(prefReverseWrapAroundKey);
    selectedReverseWrapAround = currentReverseWrapAround;
  }

  void save() {

    preferences.end();
    preferences.begin(preferencesName, false);

    preferences.putBool(prefNewlineModeKey, selectedNewLineMode);
    preferences.putBool(prefSmoothScrollKey, selectedSmoothScroll);
    preferences.putBool(prefKeyAutoRepeatKey, selectedAutoRepeat);
    preferences.putBool(prefCursorBlinkKey, selectedCursorBlink);
    preferences.putInt(prefCursorStyleKey, selectedCursorStyle());
    preferences.putBool(prefBackspaceStyleKey, selectedBackspaceStyle);
    preferences.putBool(prefWrapAroundKey, selectedWrapAround);
    preferences.putBool(prefReverseWrapAroundKey, selectedReverseWrapAround);

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

    if (!preferences.isKey(prefNewlineModeKey)) {
      preferences.putBool(prefNewlineModeKey, DEFAULT_NEWLINEMODE);
    }
    if (!preferences.isKey(prefSmoothScrollKey)) {
      preferences.putBool(prefSmoothScrollKey, DEFAULT_SMOOTH_SCROLL);
    }
    if (!preferences.isKey(prefKeyAutoRepeatKey)) {
      preferences.putBool(prefKeyAutoRepeatKey, DEFAULT_AUTOREPEAT);
    }
    if (!preferences.isKey(prefCursorBlinkKey)) {
      preferences.putBool(prefCursorBlinkKey, DEFAULT_CURSOR_BLINK);
    }
    if (!preferences.isKey(prefCursorStyleKey)) {
      preferences.putInt(prefCursorStyleKey, DEFAULT_CURSOR_STYLE);
    }
    if (!preferences.isKey(prefBackspaceStyleKey)) {
      preferences.putBool(prefBackspaceStyleKey, DEFAULT_BACKSPACE_STYLE);
    }
    if (!preferences.isKey(prefWrapAroundKey)) {
      preferences.putBool(prefWrapAroundKey, DEFAULT_WRAPAROUND);
    }
    if (!preferences.isKey(prefReverseWrapAroundKey)) {
      preferences.putBool(prefReverseWrapAroundKey, DEFAULT_REVERSE_WRAPAROUND);
    }

    preferences.putInt(prefVersionKey, preferencesVersion);

    preferences.end();
    preferences.begin(preferencesName, true);

    fetch();
  }

  void apply() {

    terminal.enableNewLineMode(currentNewLineMode);
    terminal.enableSmoothScroll(currentSmoothScroll);
    terminal.enableKeyAutorepeat(currentAutoRepeat);
    terminal.enableCursorBlinking(currentCursorBlink);
    terminal.setCursorStyle(currentCursorStyle());
    terminal.setBackarrowKeyMode(currentBackspaceStyle);
    terminal.setWrapAround(currentWrapAround);
    terminal.setReverseWrapAroundMode(currentReverseWrapAround);
  }

  void toggleNewLineMode() {
    selectedNewLineMode = !selectedNewLineMode;
  }

  void toggleSmoothScroll() {
    selectedSmoothScroll = !selectedSmoothScroll;
  }

  void toggleAutoRepeat() {
    selectedAutoRepeat = !selectedAutoRepeat;
  }

  void toggleCursorBlink() {
    selectedCursorBlink = !selectedCursorBlink;
  }

  int currentCursorStyle() {
    return selectableCursorStyles[currentCursorStyleIndex];
  }

  int selectedCursorStyle() {
    return selectableCursorStyles[selectedCursorStyleIndex];
  }

  char *selectedCursorStyleString() {
    return selectableCursorStylesStrings[selectedCursorStyleIndex];
  }

  int determineCursorStyleIndex(int cursorStyle) {
    for (int index = 0; index < NUM_CURSORSTYLES; index++) {
      if (selectableCursorStyles[index] == cursorStyle) {
        return index;
      }
    }
    return 0;
  }

  void selectNextCursorStyle() {
    selectedCursorStyleIndex++;
    if (selectedCursorStyleIndex >= NUM_CURSORSTYLES) {
      selectedCursorStyleIndex = 0;
    }
  }

  void selectPrevCursorStyle() {
    if (selectedCursorStyleIndex == 0) {
      selectedCursorStyleIndex = NUM_CURSORSTYLES - 1;
    }
    else {
      selectedCursorStyleIndex--;
    }
  }

    void toggleBackspaceStyle() {
    selectedBackspaceStyle = !selectedBackspaceStyle;
  }

  void toggleWrapAround() {
    selectedWrapAround = !selectedWrapAround;
  }

  void toggleReverseWrapAround() {
    selectedReverseWrapAround = !selectedReverseWrapAround;
  }
};

TerminalPreferences terminalPreferences;
