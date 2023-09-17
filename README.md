# Overview
IoT Conference hall mic system using ESP32-WROOM-32 for audio processing where IoT web app is used for volume controlling.

<process diagram>
INMP441 microphone module converts audio signals into I2S signal and transfered to ESP32-WROOM-32 named as Mic-ESP.This Mic-ESP is controlled by web app using a firebase server.Through Firebase,volume of eac mic can be controlled.
Then this processed I2S signal is transfered to another ESP32-WROOM-32 named as main-controller-ESP.main-controller-ESPoutputs a mixed I2S signal from each channel to MAX98357 module.This Amplifier module amplifies the signal and converted to analog signal and transfered to speaker(impedance>3Ω).

## Process Diagram
<img width="1512" alt="Screenshot 2023-09-17 at 21 17 42" src="https://github.com/IsuruVithanage/ConferenceHallMicSystem/assets/38922397/7177f9e1-bc48-4fbd-a73b-5c8a889f59bb">


This repository consists of two directories, namely:
1. Codes - Contains codes for microcontrollers of mic system and web app
2. Design - Contains files related to design

## WebApp
<SS of webapp>
<img width="1512" alt="Screenshot 2023-08-10 at 23 31 27" src="https://github.com/IsuruVithanage/ConferenceHallMicSystem/assets/38922397/216ee8a2-0cb7-4bec-ab6d-d13cb422dc35">
<img width="1512" alt="Screenshot 2023-08-10 at 23 43 28" src="https://github.com/IsuruVithanage/ConferenceHallMicSystem/assets/38922397/c1bdaac4-e795-4758-97eb-6ec084f452dc">


## 3D Design
![m3](https://github.com/IsuruVithanage/ConferenceHallMicSystem/assets/38922397/82024462-b4b0-4224-8329-baf95c24bcf0)


