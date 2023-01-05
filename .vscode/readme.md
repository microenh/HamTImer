# Ham ID Timer

![Pico-LCD](/assets/OneTimer.jpg) ![Pico-LCD](/assets/TwoTimer.jpg) 

Uses a Pico-LCD-1.14 backpack on a Raspberry Pi Pico to display one or two timers.

Per §97.119 of the FCC's Amateur Radio Regulations:  

*(a) Each amateur station, except a space station or telecommand station, must transmit its assigned call sign on its transmitting channel at the end of each communication, and at least every 10 minutes during a communication, for the purpose of clearly making the source of the transmissions from the station known to those receiving the transmissions. No station may transmit unidentified communications or signals, or transmit as the station call sign, any call sign not authorized to the station.*

In addition when giving a presentation on a repeater, the presenter needs to be aware of any
timeout timer and drop carrier to allow the timer to reset.

This project uses a Raspberry Pi Pico (no wireless required) and a LCD *backpack* to provide countdown timers. The particular code was designed for the [Waveshare 1.14inch LCD Display Module for Raspberry Pi Pico](https://smile.amazon.com/gp/product/B0948T7M9S/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&th=1), but it should work with other similar displays based on the ST7789 driver via SPI. Constants defined in the code may need to be changed if the interface uses different GPIO pins.

The *backpack* is designed to plug directly into the pins on the bottom of the Raspberry pi.

## Operation

When powered on, the display shows one or two timers counting down from user specified time limits. When the bottom timer (if displayed) reaches 0:00 **TIMEOUT** is displayed. When the top (or only) time reaches 0:00, the display flashes **ID**.

The timers can be reset at any time by pressing the A (top/only timer) or B (bottom timer) buttons to the right of the display.

## Configuration

Pressing the center joystick button to the left of the display enters a configuration menu with four options. Press the center button to move through the options.

- Timer A (top/only timer) timeout period - The up and down joystick buttons can be used to update this time in 15 second increments.
- Timer B (bottom timer) timeout period - Setting this to 0:00 disables the bottom timer. The up and down joystick buttons can be used to update this time in 15 second increments.
- Flash - This determines if the **ID** message will flash. Use the up and down joystick buttons to toggle between ON and OFF.
- BKLGT (backlight) - this controls the brightness of the backlight. Use the up and down joystick buttons to adjust. The backlight level changes as this value is adjusted.

Pressing the center joystick button after the BKLGT option saves the new values and returns to the normal timer operation. The new values will be active immediately and will also be used the next time the timer is powered up.

# Software Considerations

## Display Driver
Waveshare and the other vendors offer some C-based driver software for their devices, but only a small portion of the driver code was actually used. The largest font provided was only 24 pt and was too small. The driver also made use of a frame buffer in the Pico's RAM and transferred whole screens at a time. Updating the whole screen caused noticable flicker. 

The code keeps track of which character is displayed and only updates a rectangle containing the character when the character changes. Instead of a frame buffer in RAM, the bits in the font determine whether the foreground or background color is sent directly to the controller.

## Fonts
The [FontEdit](https://github.com/ayoy/fontedit) from GitHub was used to create larger fonts based on the open-source Liberation Mono font from the Raspbian Linux distribution. The FontEdit working files are included if the font needs to be modified. The output font data is in the .c files in the fonts directory. Only the 36 and 48 pt versions are used. To save a small amount of space, the first character in the font file (the space character) was removed and a space is drawn by clearing the appropriate area to background color.

## Non-volatile storage
Unlike some other microcontrollers, such as the Arduino, the Pico contains no EEPROM for non-volatile storage. While external EEPROM can be added, that's not convenient with the *backpack* design. Instead, a portion of the FLASH memory is used to save the settings.

FLASH is not truly READ/WRITE. Instead, the erase process resets all the bits in the specified range to 1 and programming can only change bits to 0. On the Pico, FLASH can only be erased in 4096 byte blocks and can only be programmed in 256 byte blocks. Each bit in FLASH memory can only handle a limited number of read/write cycles, compared to EEPROM. Thus, the program implements a simple wear leveling schem to extend the life of the FLASH.

The configuration data fits into 32 bits or 4 bytes. No valid setup configuration will result in either all 32 bits 1 or all 32 bits 0. A freshly erased FLASH will have all bits set to one. The program allocates the last 4096 byte block (highest memory address) to store configuration data. Instead of rewriting a new configuration, the program writes all 0 bits to the previous configuration and stores the new configuration in the next 4 bytes. When the program starts, the program skips over any 0 bytes to find the saved data. When the entire 4096 block has been used, the program erases the flash and starts over.

# Power
The Pico has a buck / boost voltage regulator that can handle voltages from 1.8 to 5.5 volts. This voltage can ge supplied via the power contacts of the micro USB connector. The current draw is minimal, allowing the timer to be powered by 2 or 3 AA or AAA batteries.



