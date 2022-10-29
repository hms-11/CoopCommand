# CoopCommand

#Development on this project has stopped. See [CoopCommand ESP32](https://github.com/hms-11/CoopCommandESP32) for the next generation controller and code.

Chickens are simple animals with lots of benefits. The downside? Humans are not the only creatures that find chickens tasty and chickens themselves require some basic maintenance and care to keep happy and healthy. 

CoopCommand aims to reduce the daily labour of looking after chickens, improve their well-being as well as allow hobby-farmers the ability to go out for the night without worrying if their chickens are in danger from wandering predators. 

Contributors always welcome, I could use people smarter than myself to keep improving this project. 

You can also support this project through "Buy me a Coffee": 

<a href="https://www.buymeacoffee.com/AutoHobbyFarm" target="_blank"><img src="https://www.buymeacoffee.com/assets/img/custom_images/orange_img.png" alt="Buy Me A Coffee" style="height: 41px !important;width: 174px !important;box-shadow: 0px 3px 2px 0px rgba(190, 190, 190, 0.5) !important;-webkit-box-shadow: 0px 3px 2px 0px rgba(190, 190, 190, 0.5) !important;" ></a>

In action: Here is a video of CoopCommand installed in my coop. https://www.youtube.com/watch?v=HNLnl-pNmuM


[![CoopCommand Picture](https://github.com/hms-11/CoopCommand/blob/main/PXL_20210510_112449138.jpg)](https://github.com)

# GOALS:

- Ease of installation and configuration for non-technical individuals. 

- Multiple Power Options (wall wart mains power or 12-36V off-grid solar/wind/battery systems).

- Configuration settings saved in case of power-outage.

- Wi-Fi connectivity for remote monitoring and overriding of coop door.

- Camera for viewing inside of coop.

# FEATURES:

## COOP COMMAND MAIN BOARD:

- Pluggable Terminal Block connectors for all user-installed inputs/outputs.

- JST XH 5-pin for connection to CoopCam Wi-FI Camera Ad-On.

- DHT22 for interior coop temperature.

- DS18B20 for water temperature.

- GL5539 Photoresistor w/10K resistor voltage divider for daylight sensor.

- MAX3373 level shifting IC for Atmega-ESP32 serial communication.

- 7805 "footprint" - Actually uses either one of the drop in footprint compatible buck convertors or my own "MEGA7805" buck converter that can output up to 3A cont. 

- TI DRV8871 Motor Driver IC w/3.6A current cabilitity & voltages up to maximimum working voltage of CoopCommand.

- 20x4 LCD.

- I2C backpack integrated.

- 3 user input buttons.

- EEPROM for saving user settings.

- LayLight Relay for supplementing daylight hours to keep chickens laying even with less than 14 hours of Daylight.

- Ventilation Fan Relay for cooling the coop in the summer.

- Water Heat Relay for heating the water in the winter. 

- ATMEGA328P in DIP configuration, easily programmed through Arduino IDE.


## ESP32 COOPCAM:

- ESP32-CAM AI-THINKER based.

- Custom PCB for ESP-32-CAM to socket into, breaks out remaining GPIO's, breaks out serial header for programming and boot button as well as on-board bulk capacity (200UF).

- JST XH 5-pin Connector, identical pinout to mainboard side to allow cable to be plugged in either way.

- Wi-Fi connection through Blynk.

- Serial connection to main board.

## BLYNK:

- Override door open or closed.

- Take a picture with the CoopCam and display it in the app

- Monitor door status (Open, Closed, Opening/Closing).

# Getting Started:

All files are included in this git-repository to get CoopCommand up and running. To get started, use the Gerber files and BOM to get the PCB's coming. The pic n place files can be used if your board house supports assembly. These boards were designed to use JLCPCB's assembly service with as many "basic" components as possible. 

Once you have the boards in hand and assembled, see the programming notes file for instructions on how to load the code. ATTENTION: This guide assumes the ATMEGA328P has a bootloader already installed, if working with new, "bare" chips you will need to burn a bootloader before installing the sketch. 

For the ESP32-CAM "CoopCam" aspect, this guide from Random Nerd Tutorials outlines the steps for setting up a gmail account with the proper settings. https://randomnerdtutorials.com/esp32-cam-send-photos-email/ 
These settings, along with your wifi info and BLYNK authentication token will need to be put into the sketch for the camera. 

# CURRENT KNOWN ISSUES:

- Some Camera ribbon cables seem to be different lengths, some tweaking of CamLid file may be needed to ensure lens fits through housing.  

- The CoopCam and associated Blynk app should not be used at this point. Reboot can cause CoopCommand to go into manual door mode, leaving the door open at night. Main board functions fine running solo. 

# PLANNED UPGRADES:


- Figure out what is up with the multi-button press sometimes required. 

- Improved EEPROM setting saves with more user settings being saved to EEPROM (currently door close light level, water heat on and ventilation fan on temps are saved).

- "self configuration" of system on startup. I would like the door to run an initial cycle on first power-on so it can self test motor connections and adjust if connected "backwards" 

- Easy deployment of CoopCam. Would like users to log into a self-hosted website on the ESP32-Cam, input their Wifi info and Blynk Authorization certificate. then have the system reboot on their network, attached to their Blynk app. 
