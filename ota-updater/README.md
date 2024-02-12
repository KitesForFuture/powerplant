# OTA-Updater based on the Raspberry Pi 4 B

The Updater runs on a Raspberry Pi with Linux installed and updates:
* VESC
* 1st ESP32 on groundstation PCB that communicates with the kite via an external WiFi antenna
* 2nd ESP32 on groundstation PCB that can be used to live-configure the kite in the field using a phone or laptop with WiFi
* ESP32 on the kite if kite is connected to the Raspberry Pi inside the groundstation via USB (only exposed USB port on groundstation with OTA-capability)

## Use the SD-Card image for the Raspi, or:

## SSH into Raspberry Pi
* Somehow succeed in getting WiFi running on the Pi..., then:
* ssh pi@raspberrypi.local

## Manually Setup of OTA Linux on Raspberry Pi
* Somehow create fixed aliases for the four physical USB sockets /dev/ttyUSB0, ..., name them esp*
* Install espressif idf as root on the Raspberry Pi
* /etc/rc.local needs the line bash -c '/root/ota/run > /home/pi/mylog.log 2>&1' &
* 
