# Emulated NES APU (Audio Processing Unit)

Implements the Pulse, Triangle, and Noise sounds of the NES

Based on specification found at https://wiki.nesdev.com/w/index.php/APU

Can play NSF files including classic Zelda I hits

Has keyboard mode, where notes are input into the device and saved

Looper to record atop previous saves

This includes a full-featured GUI for adjusting note properties
# File Contents

Player.c contains the input loop and manages calling gui functions and output sound at the aprorpiate time

APU.c contains the APU functions for setting registers and clocking

Gui.c has the GUI functions for the keyboard and the NSF player

# Player
![NES Player](https://raw.githubusercontent.com/spencerflem/NES-APU/master/keyboard.jpg)