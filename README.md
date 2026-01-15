# SporosTerm
A terminal application for the FabGL VGA32, uTerm2-S and nTerm2-S.

This works with the FabGL library, but only a patched version which can be found here: https://github.com/RetepV/FabGL/tree/FabGL-nTerm2-S

I am working on documentation, but the UI is so simple that you can figure it out yourself. Connect the nTerm2-S to a VGA screen and connect a PS/2 keyboard. When SporosTerm starts up, it's ready to work. If you want to change settings, press F12. Don't forget to press 'A' for sAve (or Apply?) to save your changes.

# How to build

This is a quick description of how to build and deploy this application using the Arduino environment. I have provided binaries to upload to the nTerm2-S, which means that you should not have to build this code by yourself, but you might want to. You can find the binaries here: https://github.com/RetepV/SporosTerm/releases.

For building, you will need:

* Arduino development environment, set up with ESP32 support. You can get that from here: https://github.com/espressif/arduino-esp32
* My own patched version of the FabGL library. You can get that from here: https://github.com/RetepV/FabGL/tree/FabGL-nTerm2-S, download it as a .ZIP file.
Note that you should not use the FabGL `main` branch, but the `FabGL-nTerm2-S` branch. This has the fixes and extensions necessary to make SporosTerm work.
* The code from this repository.

Steps:

1. Make sure to be on the latest Arduino development environment.
2. Follow the instructions at https://github.com/espressif/arduino-esp32 to install the ESP32 board support.
3. After having installed the ESP32 board support, you will have to open the Arduino Boards Manager and find the entry `esp32 by Espressif Systems`. Here, you will have to switch your installed version (probably `3.3.2` or later) to version `2.0.17`.
__It is absolutely necessary to downgrade the ESP32 board support to 2.0.17, in order to compile the FabGL library.__
4. Download the patched FabGL library's source code from here: https://github.com/RetepV/FabGL/tree/FabGL-nTerm2-S. Doublecheck if the `FabGL-nTerm2-S` branch selected. Download the source code as a .zip file, it will probably be named `FabGL-FabGL-nTerm2-S.zip`.
5. Download the SporosTerm source code from this repository, and then open `SporosTerm.ino` in the Arduino development environment.
6. In the Arduino development environment, go to `Sketch->Include Library->Add .ZIP Library`. Select the `FabGL-FabGL-nTerm2-S.zip` file, wait until Arduino finishes processing and tells that the library was installed successfully.
7. In the Arduino development environment,  go to `Sketch->Include Library->Manage Libraries` and filter on `onewire`.  Install the 'Onewire' library version 2.3.8 (by Jim Studt, Tom Pollard, Robin James and Paul Stoffregen.

Now the project is ready for building. But you will have to set up the board parameters in the Arduino development environment first.

8. Connect the nTerm2-S board and find out which serial port is created by the Operating System. I am on Macos, and that will create `/dev/cu.usbserial-DN01JQWK`. On my Linux machine, it creates `/dev/ttyUSB0`. On my Windows machine, COM5 is created.
9. Go to `Tools->Board>Esp32` and choose `ESP32 Dev Module`. This enables the `Partition Schemes` that we need later.
10. Go to `Tools->Port` and choose the port that you found in step 8.
11. Go to `Tools->Partition Scheme` and choose a scheme that has an `APP` partition of at least 1.5MB. The cheapest ESP32's have 4MB flash, choose `NO OTA (2MB APP/2MB SPIFFS)`. If you have an ESP32 with 16MB flash (like I have), you can choose `16M Flash (3MB APP/9.9MB FATFS)`. As long as the `APP` partition is larger that 1.5MB.
12. Go to `Tools->Upload Speed` and choose 921600. If you have communications problems (maybe using a low quality cable), choose something slower. It doesn't matter, you'll only have to wait a bit longer.

Now you should be ready to go!

13. Select the `->` (upoad) icon, and Arduino should build everything, upload the sketch to the nTerm2-S, and after a reset you should be greeted with the SporosTerm startup screen.

![IMG_4192](https://github.com/user-attachments/assets/4e406abc-ee41-404d-b133-55c71b090534)


>[!NOTE]
>On my Macbook, I notice that it will not always provide power to my nTerm2-S board when I connect it directly to an on-board USB port. Probably the Macbook does not trust the nTerm2-S, blacklists it, and will refuse to power it. The solution is to connect through a USB hub.
>The nTerm2-S uses no more than 250mA. With display modes that support Bluetooth, it seems to use about 150mA when transferring a lot of data. With display modes that don't support Bluetooth, it's about 80mA continuous. During boot, I can see a spike of around 250mA, probably the ESP32's radio is turned on by default during boot, and it takes a few milliseconds to shut it down again.
>Anyway, Macbooks are very expensive to repair, so it's never a bad idea to connect USB devices through a hub. :sunglasses:
>
>

# Why "Sporos"? What does it mean?

Well, this is Sporoulis, or Sporos for short. He is Greek, and Sporoulis means "little seed" in Greek. No, I'm not Greek, only Sporoulis is. ;) I just find it a great name!

![IMG_9341](https://github.com/user-attachments/assets/2c70e353-a5bb-4a54-a6bc-1fac8de8467c)

