# CoopCommand

Chickens are simple animals with lots of benefits. The downside? Humans are not the only creatures that find chickens tasty and chickens themselves require some basic maintenance and care to keep happy and healthy. 

CoopCommand aims to reduce the daily labour of looking after chickens, improve their well-being as well as allow hobby-farmers the ability to go out for the night without worrying if their chickens are in danger from wandering predators. 

# GOALS:

-Ease of installation and configuration for non-technical individuals. 

-Multiple Power Options (wall wart mains power or 12-36V off-grid solar/wind/battery systems).

-Configuration settings saved in case of power-outage.

-Wi-Fi connectivity for remote monitoring and overriding of coop door.

-Camera for viewing inside of coop.

# FEATURES:

# COOP COMMAND MAIN BOARD:

-Pluggable Terminal Block connectors for all user-installed inputs/outputs.

-JST XH 5-pin for connection to CoopCam Wi-FI Camera Ad-On.

-DHT22 for interior coop temperature.

-DS18B20 for water temperature.

-GL5539 Photoresistor w/10K resistor voltage divider for daylight sensor.

-MAX3373 level shifting IC for Atmega-ESP32 serial communication.

-7805 "footprint" - Actually uses either one of the drop in footprint compatible buck convertors or my own "MEGA7805" buck converter that can output up to 3A cont. 

-TI DRV8871 Motor Driver IC w/3.6A current cabilitity & voltages up to maximimum working voltage of CoopCommand.

-20x4 LCD.

-I2C backpack integrated.

-3 user input buttons.

-EEPROM for saving user settings.

-LayLight Relay for supplementing daylight hours to keep chickens laying even with less than 14 hours of Daylight.

-Ventilation Fan Relay for cooling the coop in the summer.

-Water Heat Relay for heating the water in the winter. 

-ATMEGA328P in DIP configuration, easily programmed through Arduino IDE.


# ESP32 COOPCAM:

-ESP32-CAM AI-THINKER based.

-Custom PCB for ESP-32-CAM to socket into, breaks out remaining GPIO's, breaks out serial header for programming and boot button as well as on-board bulk capacity (200UF).

-JST XH 5-pin Connector, identical pinout to mainboard side to allow cable to be plugged in either way.

-Wi-Fi connection through Blynk.

-Serial connection to main board.

# BLYNK:

-Override door open or closed.

-Take a picture with the CoopCam and email it.

-Monitor door status (Open, Closed, Opening/Closing).

# Getting Started:

All files are included in this git-repository to get CoopCommand up and running. To get started, use the Gerber files and BOM to get the PCB's coming. The pic n place files can be used if your board house supports assembly. These boards were designed to use JLCPCB's assembly service with as many "basic" components as possible. 

Once you have the boards in hand and assembled, see the programming notes file for instructions on how to load the code. ATTENTION: This guide assumes the ATMEGA328P has a bootloader already installed, if working with new, "bare" chips you will need to burn a bootloader before installing the sketch. 

# CURRENT KNOWN ISSUES:

-PCB for ESP32-CAM is "Backwards". All components must be soldered to backside of PCB as opposed to topside with the exception of the 5-pin JST which can be put on the proper side. Enclosure has been designed with this mistake in mind.

-Some Camera ribbon cables seem to be different lengths, some tweaking of CamLid file may be needed to ensure lens fits through housing. 

-CoopCommand user input buttons some times need to be pressed twice to change screens/adjust values. Unsure what the issue is at this time, minor usability issue. 

