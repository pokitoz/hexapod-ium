# hexapod-ium
An hexapod robot using arduino and servo motors

![20220917_185626 2777](https://user-images.githubusercontent.com/6117941/190868208-11697652-cd76-4673-8842-9c9b7dfe3679.jpg)
![Screenshot 2025-05-13 153730](https://github.com/user-attachments/assets/141fc9b0-9907-4e44-b969-7053a2d981f2)

## Hardware

- 6x Leg (3 of them must be mirrored)
- 6x Holder
- 12 servo motors: SG90 Micro Servo with screws
- 1 Arduino Uno
- PCA9865 (https://learn.adafruit.com/16-channel-pwm-servo-driver?view=all)
- Blu-tack
- Jumper cables

Need 5V for the motors board. VCC will be propagated to the main board (Arduino, ESP32,..)
You need at least 2A upper limit to move.

## Software

### Arduino

Basic Arduino program to activate all the servo.
See the pinout in the .ino file. The VCC going out of the PCA9868 should go to the 5V of the board.

### ESP32

Might need driver https://www.silabs.com/developer-tools/usb-to-uart-bridge-vcp-drivers?tab=downloads
