cmake_minimum_required(VERSION 3.12)
include(pico_sdk_import.cmake)
project(HamTimer)
pico_sdk_init()

# Subdirectory
add_subdirectory(fonts)

# Executable
add_executable(hamtimer 
    main.c
    display.c
    config.c
    flash.c
    lcd.c
)

# enable usb output, disable uart output
pico_enable_stdio_usb(hamtimer 0)
pico_enable_stdio_uart(hamtimer 1)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(hamtimer)

target_link_libraries(hamtimer
    pico_stdlib
    hardware_spi
    hardware_pwm
    hardware_flash
    fonts
)