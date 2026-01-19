
#pragma once

const char *savedGlyphsBufferFilename = "/glyphsbuffer.bin";

struct GlyphsBufferSaver {

// struct GlyphsBuffer {
//   int16_t         glyphsWidth;
//   int16_t         glyphsHeight;
//   uint8_t const * glyphsData;
//   int16_t         columns;
//   int16_t         rows;
//   uint32_t *      map;  // look at glyphMapItem_... inlined functions
// };

// GlyphsBuffer.map support functions
//  0 ..  7 : index
//  8 .. 11 : BG color (Color)
// 12 .. 15 : FG color (Color)
// 16 .. 31 : options (GlyphOptions)
// note: volatile pointer to avoid optimizer to get less than 32 bit from 32 bit access only memory

// when FONTINFOFLAGS_VARWIDTH = 0:
  //   glyphsData[] contains 256 items each one representing a single character

  static void saveGlyphsBuffer() {

    File bufferFile = LittleFS.open(savedGlyphsBufferFilename, FILE_WRITE);
    if (!bufferFile) {
      Serial.printf("Couldn't open file %s, errno: %d\n", savedGlyphsBufferFilename, errno);
      return;
    }

    terminal.withLockedTerminal([&bufferFile]() {

      fabgl::GlyphsBuffer *glyphsBuffer = terminal.getCurrentGlyphsBuffer();

      if (glyphsBuffer == nullptr) {
        Serial.printf("glyphsBuffer is null\n");
        return;
      }

      bufferFile.write((uint8_t *)&(glyphsBuffer->columns), sizeof(int16_t));
      bufferFile.write((uint8_t *)&(glyphsBuffer->rows), sizeof(int16_t));

      for (int y = 0; y < glyphsBuffer->rows; ++y) {
        volatile uint32_t *itemPtr = glyphsBuffer->map + y * glyphsBuffer->columns;
        for (int x = 0; x < glyphsBuffer->columns; ++x, ++itemPtr) {
          bufferFile.write((uint8_t *)itemPtr, sizeof(uint32_t));
        }
      }
    });

    bufferFile.close();

    // GlyphsBufferSaver::dumpGlyphsBuffer();
  }

  static void restoreGlyphsBuffer() {

    // GlyphsBufferSaver::dumpGlyphsBuffer();

    File bufferFile = LittleFS.open(savedGlyphsBufferFilename, FILE_READ);
    if (!bufferFile) {
      Serial.printf("Couldn't open file %s, errno: %d\n", savedGlyphsBufferFilename, errno);
      return;
    }

    terminal.withLockedTerminal([&bufferFile]() {

      int16_t bufferColumns;
      int16_t bufferRows;
      int16_t terminalColumns = terminal.getColumns();
      int16_t terminalRows = terminal.getRows();

      fabgl::GlyphsBuffer *glyphsBuffer = terminal.getCurrentGlyphsBuffer();

      if (glyphsBuffer == nullptr) {
        bufferFile.close();
        return;
      }

      if (bufferFile.read((uint8_t *)&bufferColumns, sizeof(int16_t)) != sizeof(int16_t)) {
        bufferFile.close();
        return;
      }
      if (bufferFile.read((uint8_t *)&bufferRows, sizeof(int16_t)) != sizeof(int16_t)) {
        bufferFile.close();
        return;
      }

      for (int y = 0; y < glyphsBuffer->rows; ++y) {
        volatile uint32_t *itemPtr = glyphsBuffer->map + y * terminalColumns;
        for (int x = 0; x < glyphsBuffer->columns; ++x, ++itemPtr) {
          if (bufferFile.read((uint8_t *)itemPtr, sizeof(uint32_t)) != sizeof(uint32_t)) {
            bufferFile.close();
            return;
          }
        }
      }
    });

    bufferFile.close();

    terminal.refresh();
  }

  static void dumpGlyphsBuffer() {

    File bufferFile = LittleFS.open(savedGlyphsBufferFilename, FILE_READ);
    if (!bufferFile) {
      Serial.printf("Couldn't open file %s, errno: %d\n", savedGlyphsBufferFilename, errno);
      return;
    }

    int16_t columns;
    int16_t rows;

    if (bufferFile.read((uint8_t *)&columns, sizeof(int16_t)) != sizeof(int16_t)) {
      Serial.printf("Couldn't read columns\n");
      bufferFile.close();
      return;
    }

    if (bufferFile.read((uint8_t *)&rows, sizeof(int16_t)) != sizeof(int16_t)) {
      Serial.printf("Couldn't read rows\n");
      bufferFile.close();
      return;
    }

    Serial.printf("Glyphs buffer (columns, rows): (%d, %d)\nBytes:\n", columns, rows);

    for (int y = 0; y < rows; ++y) {
      for (int x = 0; x < columns ; ++x) {
        uint32_t longWord;
        if (bufferFile.read((uint8_t *)&longWord, sizeof(uint32_t)) != sizeof(uint32_t)) {
          Serial.printf("Couldn't read enough data\n");
          bufferFile.close();
          return;
        }
        // Serial.printf("%08X ", longWord);
        // LSByte in the int32 is mostly the ASCII character. Sometimes a control character. Just print it.
        char c = longWord & 0xFF;
        Serial.printf("%c", (c >= 32) ? c : '.');
      }
      Serial.printf("\n");
    }

    bufferFile.close();
  }
};