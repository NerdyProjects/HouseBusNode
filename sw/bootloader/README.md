# Build and program bootloader:

First, build:
```
make
```

Connect your target to your programmer (Voltage, Ground, SWDIO, SWCLK). Start openocd in a second terminal window, keep it running:
```
sudo openocd -f interface/stlink.cfg  -c "transport select hla_swd" -f target/stm32f0x.cfg
```

Use gdb to flash the bootloader onto the target:
```
arm-none-eabi-gdb build-bl/bootloader.elf
target remote localhost:3333
load
quit
```

(These steps can be simplified - but I know these by heart - also, debugging works the same way)
