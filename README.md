# SpookyEyes
Spooky Eyes on two ESP32-TFT Displays

This project will display eyes on two synced ESP LCD boards, so they will act as a pair, like eyes do.  The make a great spooky addittion to halloween.  They also look good in masks and under veils.  The code is for use with the Arduino/ESP32 development environment.

I chose two of these from Ali Express, based on the GC9A01 TFT.

<picture>
 <source media="(prefers-color-scheme: dark)" srcset="ESPboard.png">
 <source media="(prefers-color-scheme: light)" srcset="ESPboard.png">
 <img alt="ESP" src="ESP">
</picture>

In the Arduino IDE, go to Manage Libraries, and install the following libraries:


(1) WiFiManager by tzapu

(2) TFT_eSPI by Bodmer

(3) ESP_NOW_Network by Your Name

You'll need to edit "C:\Users\User\Documents\Arduino\libraries\TFT_eSPI\User_Setup.h", but uncommenting the line with  tha is //#define GC9A01_DRIVER  - take out the two slashes, then save.  If your display is not the one shown, you'll need to figure out what lines to edit instead.

Compile and upload Clinet code to one unit, then server code to the other.  It's best to turn on the server one first.  It should just sit there showing a blue eye.  WHen you switch on the client, it starts shouting instructions to the server, which obeys, and the two will sync.

The code started life as a single piece of client server code, but got split eventually, so its a bit of a mess, but the basic are obvious, and allow for easy modifications and enhancements.
