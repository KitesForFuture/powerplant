idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS ".")

add_subdirectory(../../common/helpers helpers)
target_link_libraries(${COMPONENT_LIB} PUBLIC helpers)
