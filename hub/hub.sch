EESchema Schematic File Version 4
LIBS:hub-cache
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Connector_Generic:Conn_01x04 J1
U 1 1 5DC1718D
P 1150 1000
F 0 "J1" H 1068 575 50  0000 C CNN
F 1 "Conn_01x04" H 1068 666 50  0000 C CNN
F 2 "Connector_JST:JST_XH_B4B-XH-AM_1x04_P2.50mm_Vertical" H 1150 1000 50  0001 C CNN
F 3 "~" H 1150 1000 50  0001 C CNN
	1    1150 1000
	-1   0    0    -1  
$EndComp
Text GLabel 1500 900  2    50   Input ~ 0
CANL
Text GLabel 1500 1000 2    50   Input ~ 0
CANH
$Comp
L power:GND #PWR0101
U 1 1 5DC27407
P 1500 1100
F 0 "#PWR0101" H 1500 850 50  0001 C CNN
F 1 "GND" V 1505 972 50  0000 R CNN
F 2 "" H 1500 1100 50  0001 C CNN
F 3 "" H 1500 1100 50  0001 C CNN
	1    1500 1100
	0    -1   -1   0   
$EndComp
$Comp
L power:VCC #PWR0102
U 1 1 5DC27755
P 1500 1200
F 0 "#PWR0102" H 1500 1050 50  0001 C CNN
F 1 "VCC" V 1517 1328 50  0000 L CNN
F 2 "" H 1500 1200 50  0001 C CNN
F 3 "" H 1500 1200 50  0001 C CNN
	1    1500 1200
	0    1    1    0   
$EndComp
Wire Wire Line
	1350 900  1500 900 
Wire Wire Line
	1500 1000 1350 1000
Wire Wire Line
	1350 1100 1500 1100
Wire Wire Line
	1500 1200 1350 1200
$Comp
L Connector_Generic:Conn_01x04 J2
U 1 1 5DC29ABD
P 1150 1700
F 0 "J2" H 1068 1275 50  0000 C CNN
F 1 "Conn_01x04" H 1068 1366 50  0000 C CNN
F 2 "Connector_JST:JST_XH_B4B-XH-AM_1x04_P2.50mm_Vertical" H 1150 1700 50  0001 C CNN
F 3 "~" H 1150 1700 50  0001 C CNN
	1    1150 1700
	-1   0    0    -1  
$EndComp
Text GLabel 1500 1600 2    50   Input ~ 0
CANL
Text GLabel 1500 1700 2    50   Input ~ 0
CANH
$Comp
L power:GND #PWR0103
U 1 1 5DC29AC5
P 1500 1800
F 0 "#PWR0103" H 1500 1550 50  0001 C CNN
F 1 "GND" V 1505 1672 50  0000 R CNN
F 2 "" H 1500 1800 50  0001 C CNN
F 3 "" H 1500 1800 50  0001 C CNN
	1    1500 1800
	0    -1   -1   0   
$EndComp
$Comp
L power:VCC #PWR0104
U 1 1 5DC29ACB
P 1500 1900
F 0 "#PWR0104" H 1500 1750 50  0001 C CNN
F 1 "VCC" V 1517 2028 50  0000 L CNN
F 2 "" H 1500 1900 50  0001 C CNN
F 3 "" H 1500 1900 50  0001 C CNN
	1    1500 1900
	0    1    1    0   
$EndComp
Wire Wire Line
	1350 1600 1500 1600
Wire Wire Line
	1500 1700 1350 1700
Wire Wire Line
	1350 1800 1500 1800
Wire Wire Line
	1500 1900 1350 1900
$Comp
L Connector_Generic:Conn_01x04 J3
U 1 1 5DC2A3F2
P 1150 2400
F 0 "J3" H 1068 1975 50  0000 C CNN
F 1 "Conn_01x04" H 1068 2066 50  0000 C CNN
F 2 "Connector_JST:JST_XH_B4B-XH-AM_1x04_P2.50mm_Vertical" H 1150 2400 50  0001 C CNN
F 3 "~" H 1150 2400 50  0001 C CNN
	1    1150 2400
	-1   0    0    -1  
$EndComp
Text GLabel 1500 2300 2    50   Input ~ 0
CANL
Text GLabel 1500 2400 2    50   Input ~ 0
CANH
$Comp
L power:GND #PWR0105
U 1 1 5DC2A3FA
P 1500 2500
F 0 "#PWR0105" H 1500 2250 50  0001 C CNN
F 1 "GND" V 1505 2372 50  0000 R CNN
F 2 "" H 1500 2500 50  0001 C CNN
F 3 "" H 1500 2500 50  0001 C CNN
	1    1500 2500
	0    -1   -1   0   
$EndComp
$Comp
L power:VCC #PWR0106
U 1 1 5DC2A400
P 1500 2600
F 0 "#PWR0106" H 1500 2450 50  0001 C CNN
F 1 "VCC" V 1517 2728 50  0000 L CNN
F 2 "" H 1500 2600 50  0001 C CNN
F 3 "" H 1500 2600 50  0001 C CNN
	1    1500 2600
	0    1    1    0   
$EndComp
Wire Wire Line
	1350 2300 1500 2300
Wire Wire Line
	1500 2400 1350 2400
Wire Wire Line
	1350 2500 1500 2500
Wire Wire Line
	1500 2600 1350 2600
$Comp
L Connector_Generic:Conn_01x04 J4
U 1 1 5DC2AD76
P 1150 3200
F 0 "J4" H 1068 2775 50  0000 C CNN
F 1 "Conn_01x04" H 1068 2866 50  0000 C CNN
F 2 "Connector_JST:JST_XH_B4B-XH-AM_1x04_P2.50mm_Vertical" H 1150 3200 50  0001 C CNN
F 3 "~" H 1150 3200 50  0001 C CNN
	1    1150 3200
	-1   0    0    -1  
$EndComp
Text GLabel 1500 3100 2    50   Input ~ 0
CANL
Text GLabel 1500 3200 2    50   Input ~ 0
CANH
$Comp
L power:GND #PWR0107
U 1 1 5DC2AD7E
P 1500 3300
F 0 "#PWR0107" H 1500 3050 50  0001 C CNN
F 1 "GND" V 1505 3172 50  0000 R CNN
F 2 "" H 1500 3300 50  0001 C CNN
F 3 "" H 1500 3300 50  0001 C CNN
	1    1500 3300
	0    -1   -1   0   
$EndComp
$Comp
L power:VCC #PWR0108
U 1 1 5DC2AD84
P 1500 3400
F 0 "#PWR0108" H 1500 3250 50  0001 C CNN
F 1 "VCC" V 1517 3528 50  0000 L CNN
F 2 "" H 1500 3400 50  0001 C CNN
F 3 "" H 1500 3400 50  0001 C CNN
	1    1500 3400
	0    1    1    0   
$EndComp
Wire Wire Line
	1350 3100 1500 3100
Wire Wire Line
	1500 3200 1350 3200
Wire Wire Line
	1350 3300 1500 3300
Wire Wire Line
	1500 3400 1350 3400
$Comp
L Connector_Generic:Conn_01x04 J5
U 1 1 5DC2BDC4
P 1150 3950
F 0 "J5" H 1068 3525 50  0000 C CNN
F 1 "Conn_01x04" H 1068 3616 50  0000 C CNN
F 2 "Connector_JST:JST_XH_B4B-XH-AM_1x04_P2.50mm_Vertical" H 1150 3950 50  0001 C CNN
F 3 "~" H 1150 3950 50  0001 C CNN
	1    1150 3950
	-1   0    0    -1  
$EndComp
Text GLabel 1500 3850 2    50   Input ~ 0
CANL
Text GLabel 1500 3950 2    50   Input ~ 0
CANH
$Comp
L power:GND #PWR0109
U 1 1 5DC2BDCC
P 1500 4050
F 0 "#PWR0109" H 1500 3800 50  0001 C CNN
F 1 "GND" V 1505 3922 50  0000 R CNN
F 2 "" H 1500 4050 50  0001 C CNN
F 3 "" H 1500 4050 50  0001 C CNN
	1    1500 4050
	0    -1   -1   0   
$EndComp
$Comp
L power:VCC #PWR0110
U 1 1 5DC2BDD2
P 1500 4150
F 0 "#PWR0110" H 1500 4000 50  0001 C CNN
F 1 "VCC" V 1517 4278 50  0000 L CNN
F 2 "" H 1500 4150 50  0001 C CNN
F 3 "" H 1500 4150 50  0001 C CNN
	1    1500 4150
	0    1    1    0   
$EndComp
Wire Wire Line
	1350 3850 1500 3850
Wire Wire Line
	1500 3950 1350 3950
Wire Wire Line
	1350 4050 1500 4050
Wire Wire Line
	1500 4150 1350 4150
$Comp
L Connector_Generic:Conn_01x04 J6
U 1 1 5DC2C713
P 1150 4650
F 0 "J6" H 1068 4225 50  0000 C CNN
F 1 "Conn_01x04" H 1068 4316 50  0000 C CNN
F 2 "Connector_JST:JST_XH_B4B-XH-AM_1x04_P2.50mm_Vertical" H 1150 4650 50  0001 C CNN
F 3 "~" H 1150 4650 50  0001 C CNN
	1    1150 4650
	-1   0    0    -1  
$EndComp
Text GLabel 1500 4550 2    50   Input ~ 0
CANL
Text GLabel 1500 4650 2    50   Input ~ 0
CANH
$Comp
L power:GND #PWR0111
U 1 1 5DC2C71B
P 1500 4750
F 0 "#PWR0111" H 1500 4500 50  0001 C CNN
F 1 "GND" V 1505 4622 50  0000 R CNN
F 2 "" H 1500 4750 50  0001 C CNN
F 3 "" H 1500 4750 50  0001 C CNN
	1    1500 4750
	0    -1   -1   0   
$EndComp
$Comp
L power:VCC #PWR0112
U 1 1 5DC2C721
P 1500 4850
F 0 "#PWR0112" H 1500 4700 50  0001 C CNN
F 1 "VCC" V 1517 4978 50  0000 L CNN
F 2 "" H 1500 4850 50  0001 C CNN
F 3 "" H 1500 4850 50  0001 C CNN
	1    1500 4850
	0    1    1    0   
$EndComp
Wire Wire Line
	1350 4550 1500 4550
Wire Wire Line
	1500 4650 1350 4650
Wire Wire Line
	1350 4750 1500 4750
Wire Wire Line
	1500 4850 1350 4850
$Comp
L Connector:Screw_Terminal_01x02 J7
U 1 1 5DC2DAA4
P 2450 1050
F 0 "J7" H 2368 725 50  0000 C CNN
F 1 "Power IN" H 2368 816 50  0000 C CNN
F 2 "TerminalBlock_Phoenix:TerminalBlock_Phoenix_MKDS-1,5-2_1x02_P5.00mm_Horizontal" H 2450 1050 50  0001 C CNN
F 3 "~" H 2450 1050 50  0001 C CNN
	1    2450 1050
	-1   0    0    1   
$EndComp
$Comp
L Connector:Screw_Terminal_01x02 J8
U 1 1 5DC2E36C
P 2450 1550
F 0 "J8" H 2368 1225 50  0000 C CNN
F 1 "Power OUT" H 2368 1316 50  0000 C CNN
F 2 "TerminalBlock_Phoenix:TerminalBlock_Phoenix_MKDS-1,5-2_1x02_P5.00mm_Horizontal" H 2450 1550 50  0001 C CNN
F 3 "~" H 2450 1550 50  0001 C CNN
	1    2450 1550
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR0113
U 1 1 5DC2EBA1
P 2800 1050
F 0 "#PWR0113" H 2800 800 50  0001 C CNN
F 1 "GND" V 2805 922 50  0000 R CNN
F 2 "" H 2800 1050 50  0001 C CNN
F 3 "" H 2800 1050 50  0001 C CNN
	1    2800 1050
	0    -1   -1   0   
$EndComp
$Comp
L power:GND #PWR0114
U 1 1 5DC2F127
P 2800 1550
F 0 "#PWR0114" H 2800 1300 50  0001 C CNN
F 1 "GND" V 2805 1422 50  0000 R CNN
F 2 "" H 2800 1550 50  0001 C CNN
F 3 "" H 2800 1550 50  0001 C CNN
	1    2800 1550
	0    -1   -1   0   
$EndComp
$Comp
L power:VCC #PWR0115
U 1 1 5DC2F4D7
P 2800 1450
F 0 "#PWR0115" H 2800 1300 50  0001 C CNN
F 1 "VCC" V 2817 1578 50  0000 L CNN
F 2 "" H 2800 1450 50  0001 C CNN
F 3 "" H 2800 1450 50  0001 C CNN
	1    2800 1450
	0    1    1    0   
$EndComp
Wire Wire Line
	2800 1450 2650 1450
Wire Wire Line
	2800 1550 2650 1550
Wire Wire Line
	2800 1050 2650 1050
$Comp
L power:VCC #PWR0116
U 1 1 5DC314B1
P 2850 950
F 0 "#PWR0116" H 2850 800 50  0001 C CNN
F 1 "VCC" V 2867 1078 50  0000 L CNN
F 2 "" H 2850 950 50  0001 C CNN
F 3 "" H 2850 950 50  0001 C CNN
	1    2850 950 
	0    1    1    0   
$EndComp
$Comp
L Device:D_Schottky_Small D1
U 1 1 5DC32921
P 2750 950
F 0 "D1" H 2750 1155 50  0000 C CNN
F 1 "SK34" H 2750 1064 50  0000 C CNN
F 2 "Diode_SMD:D_SMA-SMB_Universal_Handsoldering" V 2750 950 50  0001 C CNN
F 3 "~" V 2750 950 50  0001 C CNN
	1    2750 950 
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x04 J9
U 1 1 5DC3B49C
P 1150 5300
F 0 "J9" H 1068 4875 50  0000 C CNN
F 1 "Conn_01x04" H 1068 4966 50  0000 C CNN
F 2 "Connector_JST:JST_XH_B4B-XH-AM_1x04_P2.50mm_Vertical" H 1150 5300 50  0001 C CNN
F 3 "~" H 1150 5300 50  0001 C CNN
	1    1150 5300
	-1   0    0    -1  
$EndComp
Text GLabel 1500 5200 2    50   Input ~ 0
CANL
Text GLabel 1500 5300 2    50   Input ~ 0
CANH
$Comp
L power:GND #PWR0117
U 1 1 5DC3B4A4
P 1500 5400
F 0 "#PWR0117" H 1500 5150 50  0001 C CNN
F 1 "GND" V 1505 5272 50  0000 R CNN
F 2 "" H 1500 5400 50  0001 C CNN
F 3 "" H 1500 5400 50  0001 C CNN
	1    1500 5400
	0    -1   -1   0   
$EndComp
$Comp
L power:VCC #PWR0118
U 1 1 5DC3B4AA
P 1500 5500
F 0 "#PWR0118" H 1500 5350 50  0001 C CNN
F 1 "VCC" V 1517 5628 50  0000 L CNN
F 2 "" H 1500 5500 50  0001 C CNN
F 3 "" H 1500 5500 50  0001 C CNN
	1    1500 5500
	0    1    1    0   
$EndComp
Wire Wire Line
	1350 5200 1500 5200
Wire Wire Line
	1500 5300 1350 5300
Wire Wire Line
	1350 5400 1500 5400
Wire Wire Line
	1500 5500 1350 5500
$Comp
L stepdown:MP2315-Mini-Module U1
U 1 1 5DC3CF1D
P 2950 2200
F 0 "U1" H 2950 2475 50  0000 C CNN
F 1 "MP2315-Mini-Module" H 2950 2384 50  0000 C CNN
F 2 "stepdown:MP2315-Mini" H 2950 2200 50  0001 C CNN
F 3 "" H 2950 2200 50  0001 C CNN
	1    2950 2200
	1    0    0    -1  
$EndComp
$Comp
L power:VCC #PWR0119
U 1 1 5DC3E631
P 2400 2150
F 0 "#PWR0119" H 2400 2000 50  0001 C CNN
F 1 "VCC" V 2418 2277 50  0000 L CNN
F 2 "" H 2400 2150 50  0001 C CNN
F 3 "" H 2400 2150 50  0001 C CNN
	1    2400 2150
	0    -1   -1   0   
$EndComp
Wire Wire Line
	2400 2150 2650 2150
$Comp
L power:GND #PWR0120
U 1 1 5DC41D76
P 2950 2500
F 0 "#PWR0120" H 2950 2250 50  0001 C CNN
F 1 "GND" H 2955 2327 50  0000 C CNN
F 2 "" H 2950 2500 50  0001 C CNN
F 3 "" H 2950 2500 50  0001 C CNN
	1    2950 2500
	1    0    0    -1  
$EndComp
Wire Wire Line
	2950 2500 2950 2400
$Comp
L Connector:Screw_Terminal_01x02 J10
U 1 1 5DC42D35
P 3750 2100
F 0 "J10" H 3830 2092 50  0000 L CNN
F 1 "Power OUT" H 3830 2001 50  0000 L CNN
F 2 "TerminalBlock_Phoenix:TerminalBlock_Phoenix_MKDS-1,5-2_1x02_P5.00mm_Horizontal" H 3750 2100 50  0001 C CNN
F 3 "~" H 3750 2100 50  0001 C CNN
	1    3750 2100
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0121
U 1 1 5DC435F6
P 3550 2050
F 0 "#PWR0121" H 3550 1800 50  0001 C CNN
F 1 "GND" H 3555 1877 50  0000 C CNN
F 2 "" H 3550 2050 50  0001 C CNN
F 3 "" H 3550 2050 50  0001 C CNN
	1    3550 2050
	-1   0    0    1   
$EndComp
Wire Wire Line
	3550 2100 3550 2050
Wire Wire Line
	3550 2200 3250 2200
$Comp
L Mechanical:MountingHole H1
U 1 1 5DC48A99
P 4150 850
F 0 "H1" H 4250 896 50  0000 L CNN
F 1 "MountingHole" H 4250 805 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3" H 4150 850 50  0001 C CNN
F 3 "~" H 4150 850 50  0001 C CNN
	1    4150 850 
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole H2
U 1 1 5DC48CD8
P 4150 1150
F 0 "H2" H 4250 1196 50  0000 L CNN
F 1 "MountingHole" H 4250 1105 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3" H 4150 1150 50  0001 C CNN
F 3 "~" H 4150 1150 50  0001 C CNN
	1    4150 1150
	1    0    0    -1  
$EndComp
$Comp
L Device:D_TVS_x2_AAC D3
U 1 1 5DC50C64
P 3300 3350
F 0 "D3" H 3300 3566 50  0000 C CNN
F 1 "D_TVS_x2_AAC" H 3300 3475 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 3150 3350 50  0001 C CNN
F 3 "~" H 3150 3350 50  0001 C CNN
	1    3300 3350
	1    0    0    -1  
$EndComp
Text GLabel 2850 3350 0    50   Input ~ 0
CANL
Text GLabel 3750 3350 2    50   Input ~ 0
CANH
Wire Wire Line
	3650 3350 3750 3350
Wire Wire Line
	2950 3350 2850 3350
$Comp
L power:GND #PWR0122
U 1 1 5DC534B0
P 3300 3600
F 0 "#PWR0122" H 3300 3350 50  0001 C CNN
F 1 "GND" H 3305 3427 50  0000 C CNN
F 2 "" H 3300 3600 50  0001 C CNN
F 3 "" H 3300 3600 50  0001 C CNN
	1    3300 3600
	1    0    0    -1  
$EndComp
Wire Wire Line
	3300 3600 3300 3500
$Comp
L power:VCC #PWR0123
U 1 1 5DC54589
P 2700 3900
F 0 "#PWR0123" H 2700 3750 50  0001 C CNN
F 1 "VCC" H 2717 4073 50  0000 C CNN
F 2 "" H 2700 3900 50  0001 C CNN
F 3 "" H 2700 3900 50  0001 C CNN
	1    2700 3900
	1    0    0    -1  
$EndComp
$Comp
L Device:D_TVS D2
U 1 1 5DC54C6C
P 2700 4100
F 0 "D2" V 2654 4179 50  0000 L CNN
F 1 "D_TVS" V 2745 4179 50  0000 L CNN
F 2 "Diode_SMD:D_SMA_Handsoldering" H 2700 4100 50  0001 C CNN
F 3 "~" H 2700 4100 50  0001 C CNN
	1    2700 4100
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR0124
U 1 1 5DC5529D
P 2700 4300
F 0 "#PWR0124" H 2700 4050 50  0001 C CNN
F 1 "GND" H 2705 4127 50  0000 C CNN
F 2 "" H 2700 4300 50  0001 C CNN
F 3 "" H 2700 4300 50  0001 C CNN
	1    2700 4300
	1    0    0    -1  
$EndComp
Wire Wire Line
	2700 3900 2700 3950
Wire Wire Line
	2700 4300 2700 4250
$Comp
L Device:C_Small C1
U 1 1 5DC5E6F7
P 3600 4200
F 0 "C1" H 3692 4246 50  0000 L CNN
F 1 "10µ" H 3692 4155 50  0000 L CNN
F 2 "Capacitor_SMD:C_1206_3216Metric_Pad1.42x1.75mm_HandSolder" H 3600 4200 50  0001 C CNN
F 3 "~" H 3600 4200 50  0001 C CNN
	1    3600 4200
	1    0    0    -1  
$EndComp
$Comp
L power:VCC #PWR0125
U 1 1 5DC5EC45
P 3600 4000
F 0 "#PWR0125" H 3600 3850 50  0001 C CNN
F 1 "VCC" H 3617 4173 50  0000 C CNN
F 2 "" H 3600 4000 50  0001 C CNN
F 3 "" H 3600 4000 50  0001 C CNN
	1    3600 4000
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0126
U 1 1 5DC5EF19
P 3600 4350
F 0 "#PWR0126" H 3600 4100 50  0001 C CNN
F 1 "GND" H 3605 4177 50  0000 C CNN
F 2 "" H 3600 4350 50  0001 C CNN
F 3 "" H 3600 4350 50  0001 C CNN
	1    3600 4350
	1    0    0    -1  
$EndComp
Wire Wire Line
	3600 4350 3600 4300
Wire Wire Line
	3600 4100 3600 4000
$EndSCHEMATC