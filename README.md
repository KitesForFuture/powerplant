# powerplant

All code necessary to build an airborne wind energy system.

* Paper: https://github.com/KitesForFuture/powerplant/blob/main/tailsitterkite.pdf

* Project website: https://hackaday.kitesforfuture.de/

* Youtube channel: https://www.youtube.com/@kitesforfuture577/videos

The system consists of the several components shown in this diagram:

![alt text](https://github.com/KitesForFuture/powerplant/blob/main/connectivity.jpg?raw=true)

## Kite

The kite is controlled by an ESP32 sitting on a custom PCB that holds the sensors (gyro, accelerometer, pressure), an EEPROM memory and connectors to battery, servo motors and ESCs.

* Find the ESP32 code in the subfolder "kite".
* A link to the hardware pcb layout and schematics is in the README of the "kite" folder.

## Groundstation

The groundstations main component is a Vedder Electronic Speed Controller (VESC), which is programmable in lisp. A custom PCB connects it to the battery, a pressure sensor and an ESP32 for Wifi connectivity.

* Find the ESP32 C code and the VESC lisp code in the subfolder "groundstation".
* A link to the hardware pcb layout and schematics is in the README of the "groundstation" folder.

## In flight configurator

For in flight configuration during the testing of new wings an ESP32 can be soldered to the top of the groundstation ESP32 connecting only 5V, GND and two pins for UART.

* Find the ESP32 C code in the subfolder "in_flight_config".
* Can also be used to plot debugging data live.

## Internet connection

You can control the kite via a website. We currently use kitesforfuture.de/control. To do so, flash the code of the repository "esp_internet_control" to the ESP32 that you soldered on top of the groundstation ESP32.

* Connects the kite to the internet via a Wifi hotspot.
* Can watch status and initiate kite launch and landing from anywhere in the world.
* Enables automatic launch and landing according to weather forecast.

## Simulation

In the simulation folder you find an interactive simulation that runs in your browser.

* Copy the whole simulation folder to your PC. Then open the kite_simulation.html file with a web browser
