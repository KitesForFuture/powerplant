add_library(i2c_devices
 interchip.c
 bmp280.c
 cat24c256.c
 mpu6050.c)

include_directories(../helpers)
target_link_libraries(i2c_devices PUBLIC helpers)