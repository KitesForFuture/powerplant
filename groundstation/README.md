# Groundstation for the Tailsitter Kite Wind Turbine

This code runs on the groundstation of the Tailsitter Kite Wind Turbine https://www.kitesforfuture.de/tailsitter.pdf

## Step 1: Build and Flash to ESP32
* Download and in stall espressif ide from https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html
* Run 'idf.py build' to build the project
* Run 'idf.py flash' to flash it to an ESP32


## Step 2: Flash to VESC
* Flash the file vesc-code.lisp to your VESC using the VESC-Tool under the the LISP tab inside the 'VESC Dev Tools' section. You probably need the VESC-6 firmware and a VESC-6-compatible hardware.

## Hardware interface between ESP32 and VESC
* https://oshwlab.com/benjamin.kutschan/groundstation-vesc6-esp32

## Kite
* The corresponding Kite for this groundstation can be found at https://github.com/KitesForFuture/kite/tree/figure-eight-2
