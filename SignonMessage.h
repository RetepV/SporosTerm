

// We want to render everything at approximately the same places, irrespective of the resolution. So here are
// some macros to translate positions from a source resolution (we take 640x350) to a dest resolution.
// We also need a separate macro to center text in the character screen, as for that we also need to consider
// the actual length of the text (without the escape control strings).
// It would be easier to just use precalculated values, but this way we are more or less resolution independent,
// which makes it easier if we add new resolutions.

#define RES_COMPENSATION(SOURCESIZE, DESTSIZE)  ((float)DESTSIZE / (float)SOURCESIZE)
#define TRANSLATE_RES(POS, SOURCESIZE, DESTSIZE)  ((int)((float)POS * RES_COMPENSATION(SOURCESIZE, DESTSIZE)))
#define HORIZ_CENTERED_STRING_POS(LEN, DESTSIZE) ((int)((float)DESTSIZE / 2) - ((float) LEN / 2))

struct SignonMessage {

  static void renderSignon() {

    const int sourceColumns = 80;
    const int sourceRows = 25;
    const int destColumns = displayPreferences.currentDisplayMode().columns;
    const int destRows = displayPreferences.currentDisplayMode().rows + (displayPreferences.currentDisplayMode().enableStatusBar ? 1 : 0);
    
    // Note: A bit tricky. If we use double width, >the whole line will be double width<, and the escape code for positioning will work on half the actual width. So
    //       we need to use the normal string length, but halve the horizontal resolution in chars. The string is actually 15 chars long, but aesthetically looks
    //       better if we use 13.
    renderStringAtPos(HORIZ_CENTERED_STRING_POS(13, destColumns / 2), TRANSLATE_RES(8, sourceRows, destRows), EC_DWI "SporosTerm v" EC_STR(VERSION_NUMBER) EC_NOF);
    renderStringAtPos(HORIZ_CENTERED_STRING_POS(48, destColumns), TRANSLATE_RES(12, sourceRows, destRows), "Copyright " EC_COPR " 2025 Sporos Tech, Peter de Vroomen");
    renderStringAtPos(HORIZ_CENTERED_STRING_POS(67, destColumns), TRANSLATE_RES(13, sourceRows, destRows), "Using the FabGL library, Copyright " EC_COPR " 2019-2022 Fabrizio Di Vittorio.");
    renderStringAtPos(HORIZ_CENTERED_STRING_POS(49, destColumns), TRANSLATE_RES(14, sourceRows, destRows), "Many thanks to Fabrizio Di Vittorio and Just4Fun!");

    SignonMessage::renderLogoAnimation();
  }

  static void renderPressKey() {
    const int sourceColumns = 80;
    const int sourceRows = 25;
    const int destColumns = displayPreferences.currentDisplayMode().columns;
    const int destRows = displayPreferences.currentDisplayMode().rows + (displayPreferences.currentDisplayMode().enableStatusBar ? 1 : 0);

    renderStringAtPos(HORIZ_CENTERED_STRING_POS(26, destColumns), TRANSLATE_RES(18, sourceRows, destRows), EC_BLK "-= Press a key to start =-" EC_NOF);
  }

  static void renderStringAtPos(int xPos, int yPos, char *string) {
    // Our preprocessor fails us when we want to use the EC_CURPOS macro with actual variables, so we need to use a small buffer.
    char posBuf[16];
    buildCursorPosCode(xPos, yPos, posBuf);
    terminal.write(posBuf);
    terminal.write(string);
  }

  static const int sourceXRes = 640;
  static const int sourceYRes = 350;

  static void renderLogoAnimation() {

    const int sourceRows = 25;
    const int destRows = displayPreferences.currentDisplayMode().rows + (displayPreferences.currentDisplayMode().enableStatusBar ? 1 : 0);

    int destXRes = displayPreferences.currentDisplayMode().xRes;
    int destYRes = displayPreferences.currentDisplayMode().yRes;

    const int spriteXPos = 288;
    const int spriteYPos = TRANSLATE_RES(9, sourceRows, destRows) * displayPreferences.currentDisplayMode().font->height;

    int finalSpriteXPos = TRANSLATE_RES(spriteXPos, sourceXRes, destXRes);
    int finalSpriteYPos = TRANSLATE_RES(spriteYPos, sourceYRes, destYRes);

    terminal.write(EC_ALLOCSPRITES(1));
    terminal.write(EC_DEFSPRITECOL(0,64,20,"M",255,255,255,SPOROS_TECH_MONO_LOGO_DATA));

    int animXValues[finalSpriteYPos];
    char scratch[32];

    for (double y = 0; y < finalSpriteYPos; y++) {
      animXValues[(finalSpriteYPos -1) - (int)y] = (int)(finalSpriteXPos + sin(y / 9.0) * 20);
    }

    for (int y = 0; y < finalSpriteYPos; y++) {
      sprintf(scratch, "\e_GSPRITESET0;V;0;%d;%d$", animXValues[y], y);
      terminal.write(scratch);
      vTaskDelay(20);
    }
    sprintf(scratch, "\e_GSPRITESET0;V;0;%d;%d$", finalSpriteXPos, finalSpriteYPos);
    terminal.write(scratch);  
  }
};
