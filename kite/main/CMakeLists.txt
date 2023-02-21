idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS ".")

add_subdirectory(helpers)
target_link_libraries(${COMPONENT_LIB} PUBLIC helpers)

add_subdirectory(i2c_devices)
target_link_libraries(${COMPONENT_LIB} PUBLIC i2c_devices)

add_subdirectory(control)
target_link_libraries(${COMPONENT_LIB} PUBLIC control)

add_subdirectory(pwm)
target_link_libraries(${COMPONENT_LIB} PUBLIC pwm)
