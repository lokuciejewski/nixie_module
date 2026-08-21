# Small module for standalone I2C control over Nixie Tube

This module is a small, low cost controller board for a nixie `z5740m` tube (and some compatible tubes, like `z5730`). It is based on the `ch32v003` microcontroller and is using a i2c protocol for communication with external host.
The board needs both VCC (either 3.3V or 5V) and the voltage for the tube (usually around 170V DC).

![V1 module](./media/module.jpg)

The module works in tandem with the [controller board](https://github.com/lokuciejewski/nixie_controller):

![V1 of the board with modules](./media/all.jpg)

## Building

Prerequisites are:

- GCC or other C compiler
- [ch32fun](https://github.com/cnlohr/ch32fun) stack
- GNU make

Clone the `ch32fun` into the root folder of this project. Then run `make build` to compile the firmware.

## Programming

A ch32v003 programmer is required (for example the WCH-Link-E). Connect the programmer to the VCC, GND and SWIO pins and run `make flash`.

## Communication over i2c bus

By default, this module starts with the address `0x40` (0x20 << 1, see [main.c](./main.c)).
After that, a new address can be written over the i2c which will be saved into the flash and the module will subsequently answer on the new address.

Communication over the i2c bus is a standard way, except there is no auto increment of the register when reading.

### I2c registers

This device contains the following i2c registers:

- I2cReg_Address -> `0x00` - used to set a new i2c address for the device
- I2cReg_NixieValue -> `0x01` - read or write the current digit that should be displayed
- I2cReg_HvValueLowByte -> `0x02` - read the high byte of the current High Voltage reading
- I2cReg_HvValueHighByte -> `0x03` - read the low byte of the current High Voltage reading
- I2cReg_NixiePwmValue -> `0x04` - read or write the new PWM brightness value (0-100)
- I2cReg_NixieBrightnessCompensation -> `0x05` - turn on or off brightness compensation (this reduces the brightness of the main digit if the commma is on)
