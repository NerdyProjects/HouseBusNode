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

## BUS

The CAN-Bus is not meant to operate with long stub lines - but using slew rate control, it is safe to do so.
Following CAN physical layer requirements (TI SLLA270, 01/2008), the maximum stub length should be less than 1/3rd of the critical length, which equals the down-and-back delay of the transition time, which is the longer of the rise or fall times:
| HVD230 Rs | Tplh | Tphl | 1/3 of critical length | max. length @ 250 kBaud | max. length @ 125 kBaud |
| --- |      ----  | ---- | ---------------------- | ---------- |
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

The network should be usable with ~50 nodes that can be power-fed at multiple times in the bus.
The bus cable resistance following the above approximation (120m) and taking 0.4mm/0.13mm² cable (AWG26) as a worst case candidate comes down to ~30 ohms.
Taking a worst case, default active operational current of 50 mA @ 3.3V for 50 nodes (~8 W), as well as a supply of 18 V already results in a current consumption of 450 mA, which would be too big for the given cabling.
Due to this, power consumption of nodes should be kept in mind when deploying them.
A node that is close to a powerful bus power feed can easily use the bus power to drive lighting or a doorbell with 20 W.
It is left to the deployer of the bus system to ensure that the bus voltage drop is taken into account when deploying powerful nodes.


# Software description

## Ideas

### Encryption / signature
Although not necessary in this project, I am interested in using a crypto library.
http://kmackay.ca/micro-ecc/ seems nice to use on our Cortex M0.

### CAN protocol
  Libuavcan CAN library (Cortex M0: 30 kBytes flash, 6 kBytes ram) seems nice as it supports broadcast messages as well as unicast transfers.
  Its DSL allows building arbitrary messages for different data to transmit.
  All devices need to agree on the subset of packets they want to use (easy, as otherwise they could not use them anyway).
  There is no additional network management layer necessary.
  Tooling seems good, development stable.
