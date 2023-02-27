# Autopilot for the Tailsitter Kite Wind Turbine

This code runs on the autopilot of the Tailsitter Kite Wind Turbine https://www.kitesforfuture.de/tailsitter.pdf

![alt text](https://github.com/KitesForFuture/powerplant/blob/main/kite/kite.jpg?raw=true)

## Step 0: Calibrate the gyro
* You can do this with our calibration robot or manually (for manual calibration we still need to commit some changes to the configuration GUI).

## Step 1: Build and Flash to ESP32
* Download and in stall espressif ide from https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html
* Run 'idf.py build' to build the project
* Run 'idf.py flash' to flash it to an ESP32

## Step 2: Connect to configure
* Power on kite while pointing the nose down -> kite boots into configuration mode
* Connect to the appearing Wifi Acces Point with any Wifi capable device (smartphone, notebook, etc.)
* Access 192.168.4.1/config with any web browser.
* Configure servo trim, direction. Swap left-right. Adjust PID variables. Calibrate pressure sensor. Test motor direction and thrust.

<img src="https://github.com/KitesForFuture/powerplant/blob/main/media/config_tool.jpg" align="left" width="250" >
<br>

## Autopilot Hardware
* https://oshwlab.com/benjamin.kutschan/kitepcb1_copy

## Groundstation
* The corresponding groundstation for this Kite can be found at https://github.com/KitesForFuture/groundstation

## Flight Simulator
* There's a javascript online flight simulator with essentially the same code at https://www.kitesforfuture.de/simulation17/kite_simulation.html
* Ctrl-S to download the .js-Code when you're on the page.

## Acknowledgement
* This autopilot would not exist without the awesome tutorials at http://brokking.net/
