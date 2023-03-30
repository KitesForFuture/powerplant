# This directory contains common code used by multiple of the other directories

## helpers
* math and timing

## i2c_devices
* communicate with altimeter, accelerometer, gyroscope and a memory chip

## pwm
* generate pwm signals to communicate with servo motors of the ailerons and the motor drivers of the propeller motors, also receive signals from a common radio control receiver

## RC.c
*  contains the code for wireless communication between kite and groundstation via the ESP-NOW protocol (simpler, faster, further reaching than normal Wifi)

## uart.c
* uart.c enables sending of arrays via uart
