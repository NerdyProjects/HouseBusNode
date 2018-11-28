/*
 * This is the bathroom node with following planned functionality:
 * * Humidity Sensor
 * * Ventilation controller (230V dedicated fan)
 * * Light controller (Default not so bride, optionally bright)
 * * Occupancy Indicator (Motion sensor, door sensor, "privately occupied" mode)
 *
 * Pinout:
 * PB4 - 230V Switch J46 (Bright Light)
 * PB3 - 230V Switch J47 (Fan)
 * PA0 - LDR against GND, 4k7 Pullup to 3.3V
 * PA2 - Occupancy red
 * PA3 - Occupancy yellow
 * PA4 - Occupancy green
 * PA5 - Occupancy switch
 * PA6 - Mosfet to GND (soft light)
 * PA7 - Mosfet to GND, 2k4 Pullup to Vin (piezo)
 * PB1 - Light switch
 * PB2 - Motion sensor
 * PB10 - door sensor
 * PB11
 *
 * Hardware notes:
 * Both 230V switches have only MOC3063, 2x 390 Ohms, BTA212-800B and 275V/0.6W Varistor in place
 * 
 * Indicator Panel (outside; 3x LED + Piezzo vs GND):
 * * Red LED 200 Ohm red
 * * Yellow LED 68 Ohm yellow
 * * Green LED 68 Ohm blue
 * * Piezo white
 *
 * Control Panel (inside; 2x switch + 2x LED vs GND)
 * * Red LED 200 Ohm red
 * * Yellow LED 68 Ohm yellow
 * * Switch (Occupancy, left) blue
 * * Switch (light, right) white
 */
