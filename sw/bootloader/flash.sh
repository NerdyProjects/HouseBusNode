#!/bin/sh

set -e

openocd -f interface/stlink-v2.cfg  -c "transport select hla_swd" -f target/stm32f0x.cfg &
OPENOCD_PID=$!
arm-none-eabi-gdb build-bl/bootloader.elf -x flash.gdb

cleanup()
{
    kill -9 "$OPENOCD_PID"
}
trap cleanup EXIT
