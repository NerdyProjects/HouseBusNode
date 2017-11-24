# Planung Heizungssteuerung / Elektronikbox Ersatz für Viessmann Trimatik-MC

## Konzept

Zur Einfachheit wird das bestehende Konzept der Trimatik-MC beibehalten, es wird lediglich die Elektronikbox ausgetauscht.
Dies hat mehrere Vorteile:
  * Bei Schornsteinfeger und Wartung kann das Original eingesetzt werden
  * Alle Anschlüsse sind bereits vorhanden
  * Eine Stromversorgung ist bereits vorhanden

Die hier entwickelte Schaltung soll zudem in ein Hausbus System mittels CAN-Bus eingebaut werden können.
Optional soll der Platinenentwurf auch weitere, einfache Busknoten ermöglichen:
  * Temperatursensor (Analog)
  * Temperatur/Feuchtesensor (Digital/I2C)
  * Digital-IO (z.B. Klingeltaster, Türöffner, Lichtschalter)
  * Lichtsteuerung (DC-PWM, AC-Triac)
  * weitere I2C Hardware (Display, IO-Expander, Keypad, ...)
  * Anbindung an lokale Knoten ("Busadapter") über TTL-UART oder 2-Draht halbduplex RS422/RS485/DMX

Die Bestückung bestimmt dabei die Möglichkeiten, das Platinendesign soll möglichst vielseitig sein.

## Komponenten

  * 2x 4 pin Bus-Stecker (CAN H/L, Vcc (5..24V, GND)
  * STM32F072C8T6 /072CBT6 Mikrocontroller, LQFP-48 (64 kByte flash, 16 kByte Ram) / 128 kByte flash
  * AT24C64 EEPROM (I2C)
  * 3.3V CAN Bus Transceiver sn65hvd230 (Rs ~47k nach GND, an µC für Standby Control)
  * SWD Anschluss
  * Nutzung eines Step-Down Moduls zum Auflöten/Aufstecken von <24V -> 3.3V
  * USB-Device (optional - wofür könnte das sinnvoll sein?)
  * KF120 2.54mm Rastermaß Anschlussklemmen (6,3mm tief)
  * options:
    * BME280 temp/pressure/humidity sensor (I2C, 3.3V: 3.3V GND SCL SDA (2.54mm)) -> 2x I2C Header
    * KY-024 hall sensor (3.3V, digital/analog output: DO 3.3V GND AO (2.54mm)) -> 1x Analog/Digital Header
    * RS422/RS485 communication (ISL83483 driver; e.g. to connect modified sparmatic zero radiator driver or other local nodes)
    * 4 Open Drain Schaltausgänge, Beispiel: Klingel, Licht;Optionen für SOT223 und TO220 N-Fets, mit und ohne Freilauf auf Board
      * FDT86113LZ SOT223 100V/~2A
      * IRF3708 TO220 30V /~15A
    * 4 Digitalein/ausgänge mit optionalem Schutz-Reihenwiderstand direkt an Controller
    * 2 Status LEDs
    * 40 pin Viessmann-Stecker
      * 6 Analogeingänge mit (schaltbaren) Pullups (für ~500 Ohm PTC Sensor) (ADC 0.. Vdda, 12bit, no oversampling)
      * 6 CMOS Schaltausgänge, 3,3V (Relais)
      * Nutzung der 6x16 Stufen Drehschalter auf Schalterplatine über 3x4 Matrix
      * Nutzung der LEDs der Frontplatte

## Software
  
  * Libuavcan CAN library (30 kBytes flash, 6 kBytes ram)

    
Schaltregler breakouts (11mmx20mm):
EN IN+ GND VO
(2.54mm Raster, Winkel/gerade pinheader)
