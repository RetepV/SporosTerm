# SporosTerm
A terminal application for the FabGL VGA32, uTerm2-S and nTerm2-S.

This works with the FabGL library, but only a patched version which can be found here: https://github.com/RetepV/FabGL/tree/FabGL-nTerm2-S

# How to build

This is a quick description of how to build and deploy this application using the Arduino environment. I have provided binaries to upload to the nTerm2-S, which means that you should not have to build this code by yourself, but you might want to.

You will need:

* Arduino development environment, set up with ESP32 support. You can get that from here: https://github.com/espressif/arduino-esp32
* My own patched version of the FabGL library. You can get that from here: https://github.com/RetepV/FabGL/tree/FabGL-nTerm2-S. Note that you will not use the FabGL `main` branch, but the `FabGL-nTerm2-S` branch. This has the fixes and extensions necessary to make SporoSterm work.
* This code.

Steps:

1. Download the latest Arduino development environment.
2. Follow the instructions at https://github.com/espressif/arduino-esp32 to install the ESP32 board support.
3. After having installed the ESP32 board support, you will have to open the Arduino Boards Manager and find the entry 'esp32 by Espressif Systems'. Here, you will have to switch your installed version (probably `3.3.2` at this moment) to version `2.0.17`. It is absolutely necessary to downgrade to 2.0.17, in order to compile the FabGL library.
4. Download the patched FabGL library's source code from here: https://github.com/RetepV/FabGL/tree/FabGL-nTerm2-S. Doublecheck if you have the `FabGL-nTerm2-S` branch selected. Download the source code as a .zip file, it will probably be named `FabGL-FabGL-nTerm2-S.zip`.
5. Download the SporosTerm source code and then open SporosTerm.ino in the Arduino development environment.
6. In the Arduino development environment, go to Sketch->Include Library->Add .ZIP Library. Select the `FabGL-FabGL-nTerm2-S.zip` file, wait until Arduino finishes processing and tells that the library was installed successfully.
7. In the Arduino development environment, now go to Sketch->Include Library->Manage Libraries and filter on `onewire`.  Install the 'Onewire' library version 2.3.8 (by Jim Studt, Tom Pollard, Robin James and Paul Stoffregen.

This should be enough to compile and install SporosTerm.

Some notes on the build settings:

1. For Tools->Board, select `ESP32 Dev Module`. This enables a whole bunch of Partition Schemes 
2. The app is about 1.32MB in size. The default Partition Scheme only allocates 1.2MB for the app, so the app won't fit. Therefore, in Tools->Partition Scheme you will need to choose a partition scheme with at least 1.5MB for the app. The exact scheme is up to you and your ESP32 type, but it needs storage for the app and some extra storage for the settings, which can be either SPIFFS (recommended) or FATFS.
If your ESP32 has 4MB (which is quite usual), you can choose `NO OTA (2MB APP/2MB SPIFFS)`.
I personally use ESP32-WROOM-32E MGN16 modules with 16MB flash, but also choose the `NO OTA (2MB APP/2MB SPIFFS)` scheme for release. For development use, I use `16M Flash (3M APP/9.9M FATFS)`.
