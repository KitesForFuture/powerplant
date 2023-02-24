# In flight configurator

* Used to configure PID variables and such during flight.

* With this code you can also live plot debugging data from the kite:

![alt text](https://github.com/KitesForFuture/powerplant/blob/main/example_diagram.jpg?raw=true)

## Step 1: Build and Flash to ESP32
* Download and in stall espressif ide from https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html
* Run 'idf.py build' to build the project
* Run 'idf.py flash' to flash it to an ESP32

## Step 2: Connect
* Power on kite
* Power on groundstation
* Connect to the appearing Wifi Acces Point with any Wifi capable device (smartphone, notebook, etc.)
* Access 192.168.4.1/config with any web browser.
