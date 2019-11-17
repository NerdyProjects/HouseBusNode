# Build and program bootloader:

First, build:
```
make
```

 - if 

Connect your target to your programmer, ST-LINK V2 in our case(3.3V Voltage, Ground, SWDIO, SWCLK). 

Run `./flash.sh`. 

In case something didn't work, read `flash.sh` and `flash.gdb` for detailed commands and execute them manually.
