cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(rp2350_lcd_bringup C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(rp2350_lcd_bringup
    main.c
    display.c
)

target_include_directories(rp2350_lcd_bringup PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(rp2350_lcd_bringup
    pico_stdlib
    hardware_spi
    hardware_gpio
)

pico_add_extra_outputs(rp2350_lcd_bringup)
