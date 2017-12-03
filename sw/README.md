# Home bus software

## Getting ready to go

 * Get a STM32 Toolchain, e.g. `arm-none-eabi`.
 * Get a copy of ChibiOS/RT + HAL and add the path in config.mk (use config.mk.example as a base)
 * Download libuavcan, e.g. by running
 ```git clone https://github.com/UAVCAN/libuavcan
 cd libuavcan
 git submodule update --init```
