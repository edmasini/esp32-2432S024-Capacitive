# ESP32-2432S024  Capacitive Touch LVGL WIFI & Bluetooth Development Board

![ESP32-2432S024-Board](img/board_front.jpg "ESP32-2432S024 Board front" ) 

ESP32-2432S024 is a development board designed around ESP32-WROOM-32 32-bit MCU Module, that includes:

- ESP32-WROOM-32 MCU
- 320x240 pixels TFT screen
- Capacitive touch based on CST820 chipset
- TF Card slot
- Battery connector (JST 1.24 2p Male) with IP5603 Power Bank System-On-Chip
- One RGB LED
- Speaker output based on SC8002B
- Photoresistor GT36516
- External JST 4p Female for GPIOs
- Temperature and Humidity sensors
- USB type-c connector with serial based on CH370

This is an inexpensive board if considering the number of integrated features in an all-round quality build. Many projects can work out-the-box with relative fast leadtime. However, this design appears to be produced (and copied) by multiple vendors and the documentation for this board is either difficult to find or fairly incomplete. 

The objective of this project is to provide a good starting point for developpers to create touch screen applications using this development board.

The original capacitive touch driver provided by the "official" documentation, referred by both CST820.x files, is overly simplified and incomplete, and does not cover screen all rotation configurations. The patched driver provided here, is allowing all 4 possible screen rotations.

## Requirements

This workspace is using the excellent plugin for VSCode [Platformio](https://platformio.org). It does provide streamlined and plug-and-play library configuration and file management. The Arduino IDE can also be used, but it requires manual configuration.

## Installation

Clone the repository

Set the screen rotation ``main.ccp``:
```
#define WIDTH 320 
#define HEIGHT 240
#defice ROTATION 1
```
In this example, the screen will display in landscape mode.

Upload to the board using a usb type-c cable. 
