# Hausbus Spielerei

## Short introduction

This repository holds hardware and software design for a work in progress house bus system.
It operates using a 4-wire bus using the CAN bus as well as supply of ~10 - 20 V.
It should provide a base for very simple nodes (e.g. sensors, digital actors) as well as more complex nodes (e.g. a heating system controller).


## Background

The system design started in November 2017 when the need of a better heating system controller arose in a housing project in Wurzen, Germany.
The gas heating system, a well overpowered Viessmann Atola controlled by a Viessmann Trimatik MC should be better controlled in light load conditions with options to get it better tuned to the actual needs.
A good heating system needs to be remote controllable as well as take a lot of different control inputs, in best case from every room as well as every radiator and maybe even some valves in between.

Additionally, it is nice to gather statistics about used gas, electricity and water.

When such a system is in place already, it would be great to combine it with other actors and sensors around the house: Doorbell, switches, PIR sensors, dimming of lights (without having to use expensive dimmers or even without standing up from your desk).

## Design implications

The design is driven by a mind that likes to play around with technology, gather statistics and try to optimize the world around.
The design is not meant to be used for things that require absolute reliability or fool-saveness.
The design is not meant to be used in applications or environment where security is needed.
The communication between the nodes happen without any security layer: Every node is able to set any value in any other node, so can everybody tapping into the bus wire.

# Hardware description

## Microcontroller

The microcontroller in use is the STM32F072RBT6. It is powered by an ARM Cortex-M0 core with 128 kBytes of flash memory and 16 kBytes of SRAM data memory.

### Compatible boards/projects

There is a list of boards using the same controller.
Because of the broad availability and use of these boards, they might be worth a look for application examples (be it hardware or software).

* STM32F072 Discovery Kit
* STM32 Nucleo-64 / Nucleo-F072RB (Arduino like development board)

## BUS

The CAN-Bus is not meant to operate with long stub lines - but using slew rate control, it is safe to do so.
Following CAN physical layer requirements (TI SLLA270, 01/2008), the maximum stub length should be less than 1/3rd of the critical length, which equals the down-and-back delay of the transition time, which is the longer of the rise or fall times:

| HVD230 Rs | Tplh | Tphl | 1/3 of critical length | max. length @ 250 kBaud | max. length @ 125 kBaud |
| --------- | ---- | ---- | ---------------------- | ----------------------- | ----------------------- |
| 0         | 35 ns| 70 ns | 2.3 m (1.1 m)         | ~280 m |
| 10k       | 65 ns| 130 ns | 4.3 m (2.1 m)        | ~282 m |
| 100k      | 500 ns| 870 ns | 29 m (16.6 m) | ~208 m |

The max. length is calculated using the HVD230 given tp of ~37 ns, the comparator delay inside the processor is estimated with 10 ns.
The calculation shows, that the effect of slew rate control on bus length is minimal for these speeds.
At a higher speed, the transition delay inside the driver takes up bigger parts of the available time window, which is ~3 us for 250 kBaud.
Also, it is not exactly clear to me why the speed does not matter when calculating the stub lengths, but the given results clearly show, that 100k slew rate control and a 250 kBaud bus is appropriate for usage in a house.
For a normal sized living house, the bus should be fed through each flats hallway to allow each rooms' connection with stubs.
For houses like the housing project where this is designed for, this leads to a bus length of approximately 120m:
Both ends "stub" 30m each, 2 times 3 floors with 3.5 m height (connection just through stubs, as all points are reachable within 30m), 2 times basement to pass to the next building (15m), which sums up to 30+30+2\*3\*3.5+15+15 = 111m

## Power

Each bus node has an on-board step down converter to convert the bus voltage (because of the availabiliy of cheap step downs <24 V) to 3.3 V for the circuit.
Some applications might use the bus power directly, when exact voltage does not matter and current is limited (so the bus is not powered off).
The network should be usable with ~50 nodes that can be power-fed at multiple times in the bus.

The bus cable resistance following the above approximation (120m) and taking 0.4mm/0.13mm² cable (AWG26) as a worst case candidate comes down to ~30 ohms.
Taking a worst case, default active operational current of 50 mA @ 3.3V for 50 nodes (~8 W), as well as a supply of 18 V already results in a current consumption of 450 mA, which would be too big for the given cabling.
Due to this, power consumption of nodes should be kept in mind when deploying them.

A node that is close to a powerful bus power feed can easily use the bus power to drive lighting or a doorbell with 20 W.
It is left to the deployer of the bus system to ensure that the bus voltage drop is taken into account when deploying powerful nodes.

## Board pin mapping (Board: Hausspielerei 11/2017)

Please always consider the schematics for details.
Still, the following table describes the available connectors, suggested use cases and possible use cases.

### Analog IO

Analog pins on this board are named with their corresponding usage on the Viessmann Trimatik-MC.
All of the pins in the list feature strong pullup resistors, that are all switched by /PA0 (Q1) to AVCC (3,3V)

| pin name | controller pin | available hardware | possible alternative functions |
| -------- | -------------- | ------------------ | ------------------------------ |
| IN\_ATS  | PA1            | series 0805, 2x 0805 to GND | USART2\_RTS / RS422 DE / RS485 DE, USART4\_RX |
| IN\_KTS  | PA2            | 0805 to GND | USART2\_TX / RS422 TX / RS485 TX, COMP1\_INP, COMP2\_INM, COMP2\_OUT |
| IN\_VTS  | PA3            | 0805 to GND | USART2\_RX / RS422 RX / RS485 RX, COMP2\_INP |
| IN\_STS  | PA4            | 0805 to GND | COMP2\_INM, COMP1\_INM, DAC_OUT1 |
| IN\_FB1  | PA5            | 0805 to GND | COMP1\_INM, COMP2\_INM, DAC_OUT2, SPI1\_SCK |  
| IN\_FB2  | PA6            | 0805 to GND | COMP1\_OUT, SPI1\_MISO |
| LED_A    | PA7            | series 0805 | COMP2\_OUT, SPI1\_MOSI |

### Connectors

| Connector  | Pins                           | controller pins | primary purpose | alternative purposes | conflicting with |
| ---------- | -----------------------------  | --------------- | --------------- | -------------------- | ---------------  |
| J12, J13   | VCC GND CANH CANL              |                 | Power (5-24V), bus + passthrough |     | No conflicts |
| SMPS (J3)  | NC Vin(J12/13 Vcc) GND 3,3V    |                 | SMPS module for 3,3V power supply |    | No conflicts |
| RS485 (J17)| GND A B 3,3V                   | PA1, PA2, PA3   | RS485/RS422 connection to sub boards | By bridging appropriate Pins in U1, it can be used as USART2 TX/RX/RTS or Analog inputs | DE: J9 / IN\_ATS |
| J9         | IN\_FB1 3,3V GND IN\_ATS        | PA5, PA1        | digital/analog input for hall sensor module | | IN\_ATS: RS485 DE |
| D1 (LED A) | GND LED\_A                     | PA7             | General purpose LED | IO, Analog comparator output, SPI MOSI | No conflicts |
| J16 (230V) | SW2-A SW2-B SW1-A SW1-B        | PB0 (SW2), PB11 (SW1) | Opto triac + triac controlled power switch for 230V AC | Switching, Dimming, GPIO | No conflicts |
| J6 (DIN4..1) | DIN4 DIN3 DIN2 DIN1          | PB1 PB2 PB10 PF1 | GPIO | DIN4: Analog In | No conflicts |
| D2 (LED B) | GND LED\_B                     | PA15            | General purpose LED | IO, USART2\_RX | No conflicts |
| J5 (GND)   | GND GND GND GND                |                 | | |
| J14        | SW3 GND SW4 GND                | PA8 PB3         | Open drain load switch, TIM1\_CH1, TIM2\_CH2 | 4: SPI1\_SCK | No conflicts |
| J15        | SW1 GND SW2 GND                | PB4 PB5         | Open drain load switch, TIM3\_CH1, TIM3\_CH2 | SPI1\_MISO SPI1\_MOSI | No conflicts |
| J10 (I2C)  | 3,3V GND SCL SDA               | PB6 PB7         | I2C | USART1\_TX, USART1\_RX | Shared I2C bus with J8, EEPROM |
| J8 (I2C)   | 3,3V GND SCL SDA               | PB6 PB7         | I2C | USART1\_TX, USART1\_RX | Shared I2C bus with J10, EEPROM |
| J4 (BOOT)  | GND BOOT 3,3V                  | BOOT            | bootmode selection (1-2: FLASH, 2-3: system rom bootloader) | | |
| J7 (SWD)   | SWDIO SWDCLK GND               | PA13 PA14       | SWD debug / programming connector | | No conflicts |
| J11 (USB)  | DP DM GND                      | PA12 PA11       | USB connection | COMP2\_OUT COMP1\_OUT | No conflicts |
| J18 (UART) | TX RX GND                      | PA9 PA10        | USART1\_TX USART1\_RX | TIM1\_CH2 TIM1\_CH3 | No conflicts |
| J1/J2      | see schematics                 | see schematics  | Viessmann Trimatik MC X1 | use signals individually, see schematics |

A lot of the connectors share the same physical pins.
Especially the Trimatik-MC connector shares pins with almost all other connectors, so it is not explicitly listed in the above table.
Some functionality of the Trimatik-MC might be disabled by cutting pins (or checking for electrical compatibility) - otherwise the following connectors are conflict free here as well:

  * SWD
  * I2C
  * CAN
  * J15 / Open drain switches 1, 2

When keeping minimum Trimatik-MC controller features, following connectors can never be used:
  
  * USB
  * J17 RS422/RS485
  * 230V SW1 (only by cutting J1 Pin12)

Please have a look at the schematics when wanting to use conflicting connectors at the same time, there might be options :-)

# Software description

## Ideas

## RTOS
The simplicity of each individual node does not require an operating system.
For simplicity, ease of use and improval of robustness, a simple task scheduler is highly recommended.

[ChibiOs](http://www.chibios.org) provides a simple, high performance, low footprint kernel (ChibiOS/RT) and a useful hardware abstraction layer (HAL).
Both are well tested for the used Cortex M0 controller.
The HAL provides support for almost all components of the controller:

* USB (including USB-CDC for serial connections)
* GPIO
* ADC (including using DMA transfers)
* CAN
* Timers (including PWM outputs)
* I2C

An introduction document is available: [ChibiOS/RT3.0 The ultimate Guide](http://chibios.org/dokuwiki/doku.php?id=chibios:book:start)

### Encryption / signature
Although not necessary in this project, I am interested in using a crypto library.
http://kmackay.ca/micro-ecc/ seems nice to use on our Cortex M0.

### CAN protocol
  Libuavcan CAN library (Cortex M0: 30 kBytes flash, 6 kBytes ram) seems nice as it supports broadcast messages as well as unicast transfers.

  Its DSL allows building arbitrary messages for different data to transmit.
  All devices need to agree on the subset of packets they want to use (easy, as otherwise they could not use them anyway).

  There is no additional network management layer necessary.

  Tooling seems good, development stable.
