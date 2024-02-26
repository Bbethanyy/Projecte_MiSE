View this project on [CADLAB.io](https://cadlab.io/project/27800). 

# Projecte_MiSE

**Membres del grup:** Bethany Quintero i Rossend Cera
----------------------------------------------------
## Objectiu
Dissenyar un sistema per controlar els moviments de'un Robot mitjançant un Microcontrolador MSP430.

## Elements
- Una placa base amb el Microcontrolador MSP430
- Connexió SBW o JTAG per poder programar/debugar amb una eina externa.
- Un DISPLAY LCD 
- Un Comandament format per un joystick i 1 o 2 polsadors
- El ROBOT 1(DFRobot Mcqueen plus)
- Dues LDRs
- Un mòdul UART<->Wifi

  ## Diagrama de blocs
![image](<img width="698" alt="mise" src="https://github.com/Bbethanyy/Projecte_MiSE/assets/127347516/372f9a31-2049-40fe-b2c9-2075659ffe24">)

## Components
| Ref. | Descripció| Datasheet |
| ------ | -----------------| ------------------|
|MSP430FR2355TPT|Microcontrolador|[Datasheet](https://www.ti.com/lit/ds/symlink/msp430fr2355.pdfts=1708776366062&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FMSP430FR2355%253Fqgpn%253Dmsp430fr2355)|
|THB-1841844|Joystick| [Datasheet](https://www.mouser.es/datasheet/2/240/thb-3050713.pdf)|
|HD44780U|LCD 1602|[Datasheet](https://www.sparkfun.com/datasheets/LCD/HD44780.pdf)|
|MD21605B6W-FPTLWI3| LCD I2C| [Datasheet](https://www.farnell.com/datasheets/3164276.pdf)|
|MCCOG21605B6W-SPTLYI| LCD Text | [Datasheet](https://www.farnell.com/datasheets/2021773.pdf)|
|NSL-19M51|Detector de fotos| [Datasheet](file:///C:/Users/betha/AppData/Local/Temp/817923a0-e29a-454a-975b-60c01e8ddaef_Documents%20components%20i%20dispositius-20240224.zip.aef/LDR%20NSL-19M51.pdf)|
|...| LMV324| [Datasheet](https://www.ti.com/lit/ds/symlink/lmv324.pdf)|
|...| Trimmer tyco| [Datasheet](https://www.farnell.com/datasheets/314461.pdf)|
|...| Ultrasons HC-SR04| [Datasheet](file:///C:/Users/betha/AppData/Local/Temp/49233692-ea68-4fa0-998a-116c9ef42827_Documents%20components%20i%20dispositius-20240224.zip.827/Ultrasons%20HC-SR04%20Echo%20Locator%20Making%20it%20better.pdf)|
|...| Ultrasons HC-SR05| [Datasheet](file:///C:/Users/betha/AppData/Local/Temp/bb4ae17f-42be-418a-a625-329d45a58329_Documents%20components%20i%20dispositius-20240224.zip.329/Ultrasons%20HC-SR05_Ultrasonic_Module%20VMA306%20Schema.pdf)|
|...| Ultrasons SRF04| [Datasheet](file:///C:/Users/betha/AppData/Local/Temp/166e3fce-a4a8-462c-85ff-759b14133cc4_Documents%20components%20i%20dispositius-20240224.zip.cc4/Ultrasons%20SRF04%20Technical%20Documentation.pdf)|
