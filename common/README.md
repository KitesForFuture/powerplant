# Code used by multiple of the other directories (kite, groundstation, ...)

## helpers folder
* math and timing

## i2c_devices folder
* communicate with altimeter, accelerometer, gyroscope and a memory chip

## pwm folder
* generate pwm signals to communicate with servo motors of the ailerons and the motor drivers of the propeller motors, also receive signals from a common radio control receiver

## other files
* RC.c contains the code for wireless communication between kite and groundstation via the ESP-NOW protocol (simpler, faster, further reaching than normal Wifi)
* uart.c enables sending of arrays via uart
