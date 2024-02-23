# Arduino Satellite Tracker

The purpose of this project is to automatically track satellite and perform amateur activities like voice or telemetry. Use case includes tracking of objects in the sky like the sun or perhaps a planet.

# Materials

|Quantity|Description|Link|
|---|----|----|
|1|Wemos D1 mini|
||Proto PCB and Wires/Connectors|
|2|12v 0.6rpm Torque Turbo Worm Motor 370 Right Angle|[Shopee](https://shopee.ph/%E2%9C%BFtocawe-DC-12V-0.6RPM-High-torque-Turbo-Worm-Electric-Geared-DC-Motor-GW370-Low-Speed-i.96685751.1684854369)|
|1|L298N Dual H Bridge Stepper Motor Controller (module)|[Shopee](https://shopee.ph/L298-New-Dual-H-Bridge-DC-Stepper-Motor-Drive-Controller-Board-Module-L298N-for-Arduino-stepper-motor-smart-car-robot-i.307295456.7351756627)|
|1|6mm Rigid Flange Couppler|[Shopee](https://shopee.ph/Emprichman%E2%9D%A6-3-4-5-6-7-8-10Mm-Rigid-Flange-Coupling-Motor-Guide-Shaft-Coupler-Motor-Connecto-0-0-0-0-0-i.289645126.7650415550)|
|1|6mm Rigid Shaft Coupling|[Shopee](https://shopee.ph/COD-2-3-4-5-6-7-8mm-Rigid-Shaft-Coupling-Motor-Coupler-with-Spanner-for-RC-Boat-Car-i.58547667.11732017330)|
|1|Enclosure 158x90x65 mm|[Shopee](https://shopee.ph/JANE-ABS-Enclosure-Box-Plastic-Electronic-Boxes-Project-Instrument-Case-Parts-Accessories-Waterproof-Housing-Grey-White-Outdoor-Junction-Holder-i.50706257.9104114861)|
|1|Tripod|[Shopee](https://shopee.ph/Selfie-Ring-light-Stand-Only-i.66518620.7158120434)|

Referencce: [My Google Sheet List](https://docs.google.com/spreadsheets/d/1cMQumLH8W8JSPdNhxq0K1dAPo-xYCWljrY0g2Y8n2QI)

Wiring for L298N
--------------------
    L297N                     Wemos D1 mini
    P1 ---------------------- D5
    P2 ---------------------- D6
    P3 ---------------------- D7
    P4 ---------------------- D8

![Alt text](https://github.com/kerpz/ArduinoSatelliteTracker/blob/main/images/wiring.jpg "Wiring")

# Building / Compiling

Code Editor
- Visual Studio Code <Microsoft>

Visual Studio Code Extensions
- Arduino <Microsoft>
- C/C++ <Microsoft>
- C/C++ Extension Pack <Microsoft>
- C/C++ Themes <Microsoft>
- CMake Tools <Microsoft>

Uploadable Binary File via Webmin
- build/ArduinoStalliteTracker.ino.bin

Alternative (the easy way)
- Arduino IDE

# Device Web Screenshots
![Alt text](https://github.com/kerpz/ArduinoSatelliteTracker/blob/main/images/Screenshot_20240223_172631_Chrome.jpg "Main Page")
![Alt text](https://github.com/kerpz/ArduinoSatelliteTracker/blob/main/images/Screenshot_20240223_172711_Chrome.jpg "Config Page")
![Alt text](https://github.com/kerpz/ArduinoSatelliteTracker/blob/main/images/Screenshot_20240223_172840_Chrome.jpg "Tracker Page")
![Alt text](https://github.com/kerpz/ArduinoSatelliteTracker/blob/main/images/Screenshot_20240223_172920_Chrome.jpg "App Page")

# Device Actual Photos
![Alt text](https://github.com/kerpz/ArduinoSatelliteTracker/blob/main/images/20240223_172119.jpg)
![Alt text](https://github.com/kerpz/ArduinoSatelliteTracker/blob/main/images/20240223_172133.jpg?raw=true)
![Alt text](https://github.com/kerpz/ArduinoSatelliteTracker/blob/main/images/20240222_111427.jpg)
![Alt text](https://github.com/kerpz/ArduinoSatelliteTracker/blob/main/images/20240222_111456.jpg)

# TODO

- MCU Flasher (like NodeMCU Flasher)
- User manual
- IMU integration
- GPS integration

# Reference

[Sarcnet mk1b](https://www.sarcnet.org/rotator-mk1.html)
