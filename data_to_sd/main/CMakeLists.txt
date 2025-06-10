idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS ".")

add_subdirectory(../../common/helpers helpers)
target_link_libraries(${COMPONENT_LIB} PUBLIC helpers)

add_subdirectory(../../common/i2c_devices i2c_devices)
target_link_libraries(${COMPONENT_LIB} PUBLIC i2c_devices)
