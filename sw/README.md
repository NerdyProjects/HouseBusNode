# Home bus software

## Getting ready to go

 * Get a STM32 Toolchain, e.g. `arm-none-eabi`.
 * Get a copy of ChibiOS/RT + HAL (Development done with 17.6.3)and add the path in config.mk (use config.mk.example as a base)
 * Download libuavcan, currently you have to use the fork at https://github.com/NerdyProjects/libuavcan/tree/stm32f0
 ```git clone https://github.com/NerdyProjects/libuavcan.git
 cd libuavcan
 git submodule update --init```
