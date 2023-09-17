# Overview
IoT Conference hall mic system using ESP32-WROOM-32 for audio processing where IoT web app is used for volume controlling.

<process diagram>
INMP441 microphone module converts audio signals into I2S signal and transfered to ESP32-WROOM-32 named as Mic-ESP.This Mic-ESP is controlled by web app using a firebase server.Through Firebase,volume of eac mic can be controlled.
Then this processed I2S signal is transfered to another ESP32-WROOM-32 named as main-controller-ESP.main-controller-ESPoutputs a mixed I2S signal from each channel to MAX98357 module.This Amplifier module amplifies the signal and converted to analog signal and transfered to speaker(impedance>3Ω).


This repository consists of two directories, namely:
1. Codes - Contains codes for microcontrollers of mic system and web app
2. Design - Contains files related to design

## Codes
<SS of webapp>

## Design
<picture of 3D design>

