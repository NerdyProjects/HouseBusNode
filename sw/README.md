# HouseBus software

## Compilation

Run `make` to build the HouseBus software.
You need to specify a node type, for example:

```
make NODE=boiler
```

Existing node types are defined inside `Makefile`.

Note: to get the default node build, use `NODE=default`.

Note: you need to manually uncomment some lines in `transfer_registrations.h` depending on the node type.
This should be automated at some point.

## Upload

This assumes the node already has a bootloader.
Check [bootloader/README.md](bootloader/README.md) for instructions.

To connect your computer to the node, you need a CAN-PC interface. We use the [candleLight firmware](https://github.com/candle-usb/candleLight_fw).

Plug in the USB connector and type:

```
sudo ip link set can0 type can bitrate 125000
sudo ip link set dev can0 up
```

Connect your the node that you want to upload to.

Then use `uavcan_gui_tool` or `upload.py` to upload the software:

```
python upload.py --bus can0 --node-id 14 build/default/default.bin
```

## Upload via Ethernet

It's convenient to have a computer always connected to the HouseBus, to act as a internet gateway/log writer.
With [cannelloni](https://github.com/mguentner/cannelloni) you can tunnel CAN over ethernet.

On your computer (client):

```
sudo modprobe vcan
sudo ip link add name vcan0 type vcan
sudo ip link set dev vcan0 up
cannelloni -S c -R 192.168.178.58 -t 10000
```

Note: only one client can be connected!
