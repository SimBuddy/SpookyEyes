# SpookyEyes
Spooky Eyes on two ESP32-TFT Displays.

<picture>
 <source media="(prefers-color-scheme: dark)" srcset="eyes.png">
 <source media="(prefers-color-scheme: light)" srcset="eyes.png">
 <img alt="ESP" src="ESP">
</picture>

This project will display eyes on two synced ESP LCD boards, so they will act as a pair, like eyes do.  The make a great spooky addittion to halloween.  They also look good in masks and under veils.  The code is for use with the Arduino/ESP32 development environment.

I chose two of these from Ali Express, based on the GC9A01 TFT.

<picture>
 <source media="(prefers-color-scheme: dark)" srcset="ESPboard.png">
 <source media="(prefers-color-scheme: light)" srcset="ESPboard.png">
 <img alt="ESP" src="ESP">
</picture>

These are "ESP32 DEV" boards, so choose that when selecting a board in the IDE.

In the Arduino IDE, go to Manage Libraries, and install the following libraries:


(1) WiFiManager by tzapu

(2) TFT_eSPI by Bodmer

(3) ESP_NOW_Network by Your Name

You'll need to edit "C:\Users\User\Documents\Arduino\libraries\TFT_eSPI\User_Setup.h", by:

(1) Uncommenting the line that says: //#define GC9A01_DRIVER  - take out the two slashes.  If your display is not the one pictured above, you'll need to figure out what lines to edit instead.

(2) Further down the file, look for your a line saying :"###### EDIT THE PIN NUMBERS IN THE LINES FOLLOWING TO SUIT YOUR ESP32 SETUP   ######" - edit the pin numbers in the file, as below.

Use these working pinouts ONLY for the display device shown above - other devices will have differing pinouts:

 #define TFT_MOSI 15 // In some display driver board, it might be written as "SDA" and so on.

 #define TFT_SCLK 14

 #define TFT_CS   5  // Chip select control pin

 #define TFT_DC   27  // Data Command control pin

 #define TFT_RST  33  // Reset pin (could connect to Arduino RESET pin)

 #define TFT_BL   32  // LED back-light


(3) **I have added a working user_setup.h file which will ONLY work on the LCD display device shown above.**

Ensure you have enabled PSRAM if using the board above - do this in the Arduin IDE.

RESTART THE ARDUINO IDE TO ENABLE CHANGES.

Compile and upload client code to one unit, then server code to the other.  It's best to turn on the server one first.  It should just sit there showing a blue eye, listening for instructions.  When you switch on the client, it will start shouting instructions to the server, which obeys, and the two should sync.  This is dnoe using the ESP-NOW protocol.

The code started life as a single piece of client server code, but got split eventually, so its a bit of a mess, but the basics are obvious, and allow for easy modifications and enhancements.
