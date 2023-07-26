add_library(i2c_devices
 interchip.c
 dps310.c
 cat24c256.c
 mpu6050.c
 mpu9250.c)

include_directories(../helpers)
target_link_libraries(i2c_devices PUBLIC helpers)
