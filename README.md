# hexapod-ium

An hexapod robot using arduino and servo motors.
Manufacturing (PCB, 3D prints) and component assembly (PCBA) were done with PCBWay: https://www.pcbway.com/.

![PXL_20250726_183029919](https://github.com/user-attachments/assets/f495f0f5-c5ed-42a1-998d-fa5a84e291cc)
![20220917_185626 2777](https://user-images.githubusercontent.com/6117941/190868208-11697652-cd76-4673-8842-9c9b7dfe3679.jpg)

## Hardware

See the Kicad project in hw/ folder.

There is a Kicad Pluggin (https://www.pcbway.com/blog/News/PCBWay_Plug_In_for_KiCad_3ea6219c.html)
<img width="205" height="64" alt="image" src="https://github.com/user-attachments/assets/2eeb5994-6ae4-4f47-8fc8-b34a461be5a4" />, 
simplifying the order process with PCBWay as the Gerber and BOM are filled in.

The design is reviewed and manufactured rapidely; which was very useful for this project.

### PCB

Kicad was used for the PCB design to add 2xPCA9865 and an ESP32.

<img width="467" height="371" alt="image" src="https://github.com/user-attachments/assets/0c4a3653-8ac7-4267-8218-153bd54e0733" />

### 3D Parts 

3D Printed parts can be done using a Prusa or Bamboo or any other 3d printer like the first iteration. 
But it was simpler to use PCBWay and the quality was better than printing on my own. The parameter used were:
- Resin Standard white material (UTR 8360)
- SLA

Design check and delivery was fast.

![Screenshot 2025-05-13 153730](https://github.com/user-attachments/assets/141fc9b0-9907-4e44-b969-7053a2d981f2)

### Components

- 6x Leg (3 of them must be mirrored)
- 6x Holder
- 12 servo motors: MG90S or SG90 Micro Servo with screws
- 1 Arduino Uno
- PCA9865 (https://learn.adafruit.com/16-channel-pwm-servo-driver?view=all)
- Blu-tack
- Jumper cables

Need 5V for the motors board. VCC will be propagated to the main board (Arduino, ESP32,..)

You will need at least 2A upper limit to move all the motors at once.

## Software

### Arduino

Basic Arduino program to activate all the servo.
See the pinout in the .ino file. The VCC going out of the PCA9868 should go to the 5V of the board.

### ESP32

Might need driver https://www.silabs.com/developer-tools/usb-to-uart-bridge-vcp-drivers?tab=downloads
