
//                   COOP COMMAND V2.0 (Full Function Field Test Ready)
//              First version of Coop Command, Chicken Coop Control Software.
//
//
//                                      Component/Feature Notes:
//                          -- Photo Resistor GL5539 W/10K Divider
//                          -- Interior Temp/Humidity Sensor DHT22
//                          -- Water Temp Sensor DS18B20
//                          -- 3 User Input Buttons
//                          -- I2C Connector for 20x4 LCD Display
//                          -- User selectable settings



#include <DallasTemperature.h>
#include <OneWire.h>
#include <Wire.h>
#include <DHT.h>;
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4); // Set I2C Address and display size

// pin assignments
const int photocellPin = A0;    // analog input pin for photocell
const int button1 = 2;
const int button2 = 3;
const int button3 = 4;
const int bottomSwitchPin = A2;              // bottom switch is connected to pin A2
const int topSwitchPin = A1;                 // top switch is connected to pin A1
const int directionCloseCoopDoorMotorB = 8;  // direction close motor b - pin 8
const int directionOpenCoopDoorMotorB = 6;   // direction open motor b - pin 9
const int layLightRelay = 10; //output pin controlling lay light relay
const int fanRelay = 11; // output pin controlling ventilation fan relay
const int fanLED = 5; // output pin for ventilation fan LED indicator
const int motorLED = 7; // output pin for ventilation fan LED indicator
const int heatRelay = 9; // output pin controlling water heater relay
const int heatLED = A3; // output pin controlling water heater LED indicator

// Data wire is plugged into pin 12
#define ONE_WIRE_BUS 12
/********************************************************************/
// Setup a oneWire instance to communicate with any OneWire devices
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
/********************************************************************/
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);
/********************************************************************/
#define DHTPIN 13     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino

// Menu Variables and States

unsigned long ds18b20Delay = 2000; //delay so sensor is read every two seconds
unsigned long lastDs18b20ReadingTime = 0; // the last time the DS18B20 sensor was read
unsigned long dhtDelay = 2000; //delay so sensor is read every 2 seconds
unsigned long lastDhtReadingTime = 0; // the last time the DHT22 was read
bool menuOn = true; // state of the display menu
const bool serialDisplay = true;
bool doorOpen = true; // is the coop door open or closed
bool doorOpenOverRide = false; // is the open door override active
bool doorCloseOverRide = false; // is the door close override actice
bool ventOn = false; // is the ventilation fan relay on or off
bool heaterOn = false; // is the water heater relay on or off
bool nightTimer = false; // is it night time
bool layLightOn = true; // is the Lay Light time monitoring system on
bool nightLightOn = false; // is the Night Light
int optionSelect = 0; //which menu option is selected
int lastOptionSelect = 0;
int lastItemSelect = 0;
int itemSelect = 0;
int coopTemp = 0;
int closeDoor = 30; // Light level to close coop door (user editable)
int hotTemp = 30; //Temperature to turn on ventilation fan (user editable)
int coldTemp = 3;
int waterTemp = 0;
int chk;
float hum;  //Stores humidity value
float temp; //Stores temperature value
int photocellReading;             // analog reading of the photocell
int photocellReadingLevel = '2';        // photocell reading levels (night, light, twilight)
unsigned long layLightTimer = 36000000; // Timer to make sure at least 14 hours or "daylight"
unsigned long lastDayLightReadingTime = 0; // timer to keep track of how long it has been night
unsigned long nightLightDelay = 300000; // 5 minute timer to turn on the coop light if "enter" is pushed and it is night.
unsigned long lastNightLightTime = 0;
unsigned long photocellReadingDelay = 600000;     // 600000 = 10 minute
unsigned long lastPhotocellReadingTime = 0;       // the last time the photocell was read
int buttonState1 = 0; // current state of each button
int buttonState2 = 0;
int buttonState3 = 0;
int lastButtonState1 = 0;     // previous state of each button
int lastButtonState2 = 0;
int lastButtonState3 = 0;
int topSwitchState; // Current state (open/closed) of the top limit switch
int bottomSwitchState; // Current state (open/closed) of the bottom limit switch
unsigned long displayTimer = 6000; // timer to automatically turn off the display
unsigned long lastDisplayTimer = 0;



void setup() {
  // put your setup code here, to run once:
  dht.begin();
  sensors.begin();
  pinMode(photocellPin, INPUT);
  pinMode(button1, INPUT);
  pinMode(button2, INPUT);
  pinMode(button3, INPUT);
  pinMode(topSwitchPin, INPUT);
  pinMode(bottomSwitchPin, INPUT);
  pinMode(layLightRelay, OUTPUT);
  pinMode(fanRelay, OUTPUT);
  pinMode(fanLED, OUTPUT);
  pinMode(motorLED, OUTPUT);
  pinMode(heatLED, OUTPUT);
  pinMode(heatRelay, OUTPUT);
  pinMode(directionCloseCoopDoorMotorB, OUTPUT);
  pinMode(directionOpenCoopDoorMotorB, OUTPUT);
  lcd.begin();
  lcd.clear();
  lcd.home();
  lcd.print(" Coop Command");
  lcd.setCursor(0, 1);
  lcd.print(" Control Center");
  lcd.setCursor(0, 2);
  lcd.print(" V2.0");
  lcd.setCursor(0, 3);
  lcd.print(" LOADING...");
  delay(2000);
  lcd.clear();
  photocellReading = analogRead(photocellPin);
}



void ventFan() {
  if ((unsigned long)(millis() - lastDhtReadingTime) >= dhtDelay) {
    lastDhtReadingTime = millis();
    hum = dht.readHumidity();
    temp = dht.readTemperature();
    coopTemp = temp;
    if (coopTemp >= hotTemp) { // if the temperature is above the Hot temperature
      digitalWrite(fanRelay, HIGH);
      digitalWrite(fanLED, HIGH);
      ventOn = true;
    }
    else if (coopTemp < (hotTemp - 2)) { // if the temperature has been lowered two degrees
      digitalWrite(fanRelay, LOW);
      digitalWrite(fanLED, LOW);
      ventOn = false;
    }
  }
}

void layLight() {
  if (layLightOn) {
    if (!nightTimer) { // if it is not dark
      lastDayLightReadingTime = millis();
      digitalWrite(layLightRelay, LOW); // turn off the lay light
    }
    if (nightTimer) { // if it is dark
      if ((unsigned long)(millis() - lastDayLightReadingTime) >= layLightTimer) { //if it has been dark more than 10 hours (or whatever the timer is
        digitalWrite(layLightRelay, HIGH); // turn on the lay light
      }
      else {
        digitalWrite(layLightRelay, LOW); // turn off the lay light
      }
    }
  }
}

void nightLight() {
  if (nightLightOn) { // if someone wants the light on
    digitalWrite(layLightRelay, HIGH);
  }
  if ((unsigned long)(millis() - lastNightLightTime) >= nightLightDelay) {
    digitalWrite (layLightRelay, LOW);
    nightLightOn = false;
  }
}

void waterHeat() {
  if ((unsigned long)(millis() - lastDs18b20ReadingTime) >= ds18b20Delay) {
    lastDs18b20ReadingTime = millis();
    sensors.requestTemperatures();
    waterTemp = sensors.getTempCByIndex(0);
    if (waterTemp >= (coldTemp + 3)) { // if the temperature is 3 degrees above the trigger temp
      digitalWrite(heatRelay, LOW); //turn off the water heater
      digitalWrite(heatLED, LOW); // turn off the LED indicator
      heaterOn = false;
    }
    else if (waterTemp < coldTemp) { //if the temperature is below the cold temperature
      digitalWrite(heatRelay, HIGH); //turn on the water heater
      digitalWrite(heatLED, HIGH); // turn on the LED indicator
      heaterOn = true;
    }
  }
}

void photoCell() { // function to be called repeatedly - per coopPhotoCellTimer set in setup

  if ((unsigned long)(millis() - lastPhotocellReadingTime) >= photocellReadingDelay) {
    photocellReading = analogRead(photocellPin);
    lastPhotocellReadingTime = millis();

    //  set photocell threshholds
    if (photocellReading >= 0 && photocellReading <= closeDoor) { // Night Setting based on user or default selected low light trigger
      photocellReadingLevel = '1';
      nightTimer = true;
    }

    else if (photocellReading  >= closeDoor && photocellReading <= 125) { // Twighlight setting
      photocellReadingLevel = '2';
      nightTimer = true;
    }

    else if (photocellReading  >= 126 ) { //Daylight Setting
      photocellReadingLevel = '3';
      nightTimer = false;
    }
  }
}


// stop the coop door motor and put the motor driver IC to sleep (power saving)
void stopCoopDoorMotorB() {
  digitalWrite (directionCloseCoopDoorMotorB, LOW);      // turn off motor close direction
  digitalWrite (directionOpenCoopDoorMotorB, LOW);       // turn off motor open direction
  digitalWrite(motorLED, LOW);
}



// close the coop door motor
void closeCoopDoorMotorB() {
  if (bottomSwitchState == 1) {                         //if the bottom reed switch is open
    digitalWrite (directionCloseCoopDoorMotorB, HIGH);     // turn on motor close direction
    digitalWrite (directionOpenCoopDoorMotorB, LOW);       // turn off motor open direction
    digitalWrite(motorLED, HIGH);
  }
  else if (bottomSwitchState == 0) {                     // if bottom reed switch circuit is closed
    stopCoopDoorMotorB();
    doorOpen = false;
    doorCloseOverRide = false;
  }
}



// open the coop door
void openCoopDoorMotorB() {
  if (topSwitchState == 1) {                         //if the top reed switch is open
    digitalWrite(directionCloseCoopDoorMotorB, LOW);       // turn off motor close direction
    digitalWrite(directionOpenCoopDoorMotorB, HIGH);       // turn on motor open direction
    digitalWrite(motorLED, HIGH);
  }
  else if (topSwitchState == 0) {                            // if top reed switch circuit is closed
    stopCoopDoorMotorB();
    doorOpen = true;
    doorOpenOverRide = false;
  }
}


void readSwitches() {
  topSwitchState = (digitalRead(topSwitchPin));
  bottomSwitchState = (digitalRead(bottomSwitchPin));
}

// do the coop door
void doCoopDoor() {
  if (photocellReadingLevel  == '1') {              // if it's dark
    if (photocellReadingLevel != '2') {             // if it's not twilight
      if (photocellReadingLevel != '3') {           // if it's not light
        readSwitches();
        closeCoopDoorMotorB();                      // close the door
      }
    }
  }
  if (photocellReadingLevel  == '3') {              // if it's light
    if (photocellReadingLevel != '2') {             // if it's not twilight
      if (photocellReadingLevel != '1') {           // if it's not dark
        readSwitches();
        openCoopDoorMotorB();                       // Open the door
      }
    }
  }
  else if (photocellReadingLevel == '2') {          // if it's twilight
    if (photocellReadingLevel != '3') {             // if it's not light
      if (photocellReadingLevel != '1') {           // if it's not dark
        readSwitches();
        stopCoopDoorMotorB();
      }
    }
  }
}


void doorOverRide() {
  if (doorCloseOverRide) {
    closeCoopDoorMotorB();
  }
  if (doorOpenOverRide) {
    openCoopDoorMotorB();
  }
}

void readButtons() {
  buttonState1 = (digitalRead(button1));
  buttonState2 = (digitalRead(button2));
  buttonState3 = (digitalRead(button3));
  if (buttonState1 != lastButtonState1) {
    if (buttonState1 == LOW) { //if the enter/back button has been pressed
      lastDisplayTimer = millis();
      if ((itemSelect == 0) && (optionSelect != 0)) { //if we are not in the settings change
        itemSelect = 1;
      }
      else if (itemSelect == 1) {
        itemSelect = 0;
      }
    }
    lastButtonState1 = buttonState1;
  }
  if (itemSelect == 0) { // if we are not adjusting settings

    if (buttonState2 != lastButtonState2) { //if the right button has been pressed
      if (buttonState2 == LOW) { //if the right button has been pressed
        lastDisplayTimer = millis();
        if ((optionSelect <= 5) && (optionSelect != 5)) { //if we are not past the last menu screen
          optionSelect ++;
        }
        else if (optionSelect == 5) { // if we are at the last menu screen
          optionSelect = 0;
        }
      }
      lastButtonState2 = buttonState2;
    }
    if (buttonState3 != lastButtonState3) { //if the left button has been pressed
      if (buttonState3 == LOW) { //if the left button has been pressed
        lastDisplayTimer = millis();
        if (optionSelect > 0) { //if we are not at the first menu screen
          optionSelect --;
        }
        else if (optionSelect == 0) {
          optionSelect = 5;
        }
      }
      lastButtonState3 = buttonState3;
    }
  }

  else if (itemSelect == 1) { // if we are adjusting settings
    buttonState2 = (digitalRead(button2));
    buttonState3 = (digitalRead(button3));

    if (buttonState2 != lastButtonState2) { //if the right button has been pressed
      if (buttonState2 == LOW) { //if the right button has been pressed
        lastDisplayTimer = millis();
        if (optionSelect == 1) { //if we are adjusting the Vent Fan Temp
          hotTemp = (hotTemp + 5);
        }
        else if (optionSelect == 2) { // if we are adjusting the Water Heater Temp
          coldTemp = (coldTemp + 1);
        }
        else if (optionSelect == 3)  { // if we are adjusting the Door Light Sensor
          closeDoor = (closeDoor + 5);
        }
      }
      lastButtonState2 = buttonState2;
    }
    if (buttonState3 != lastButtonState3) { //if the left button has been pressed
      if (buttonState3 == LOW) { //if the left button has been pressed
        lastDisplayTimer = millis();
        if (optionSelect == 1) { //if we are adjusting the Vent Fan Temp
          hotTemp = (hotTemp - 5);
        }
        else if (optionSelect == 2) { // if we are adjusting the Water Heater Temp
          coldTemp = (coldTemp - 1);
        }
        else if (optionSelect == 3)  { // if we are adjusting the Door Light Sensor
          closeDoor = (closeDoor - 5);
        }
      }
      lastButtonState3 = buttonState3;
    }
    if (optionSelect == 4) { // if we are overriding the coop door
      if (doorOpen) { // if the coop door is open
        doorCloseOverRide = true;
        itemSelect = 0;
        optionSelect = 0; //back to front screen
        lastPhotocellReadingTime = millis();
        lastDisplayTimer = millis();
        photocellReadingLevel = '1';

      }
      else if (!doorOpen) { //if the coop door is closed
        doorOpenOverRide = true;
        itemSelect = 0;
        optionSelect = 0; //back to front screen
        lastDisplayTimer = millis();
        lastPhotocellReadingTime = millis();
        photocellReadingLevel = '3';
      }
    }
    if (optionSelect == 5) { // if we are turning the laylight option on/off
      if (layLightOn) { // if the lay light option is on
        itemSelect = 0;
        optionSelect = 0; //back to front screen
        lastDisplayTimer = millis();
        layLightOn = false;
      }
      else if (!layLightOn) { //if the lay light option is off
        itemSelect = 0;
        optionSelect = 0; //back to front screen
        lastDisplayTimer = millis();
        layLightOn = true;
      }
    }
  }
}

void displayMenu() {
  if (menuOn) { // if the display is supposed to be on
    lcd.display(); // turn the display on
    lcd.backlight();

    if (optionSelect != lastOptionSelect) {
      lcd.clear();
      lastOptionSelect = optionSelect;
    }
    else if (itemSelect != lastItemSelect) {
      lcd.clear();
      lastItemSelect = itemSelect;
    }

    switch (optionSelect) {

      case 0:
        lcd.home();
        lcd.print("    Coop Command");
        lcd.setCursor(0, 1);
        lcd.print("<                  >");
        lcd.setCursor(0, 2);
        lcd.print("   Control Center");
        lcd.setCursor(0, 3);
        lcd.print("   Firmware V2.0");
        break;

      case 1:
        if (itemSelect == 0 && ventOn == false) {
          lcd.home();
          lcd.print("  Coop Temp: ");
          lcd.print(coopTemp);
          lcd.print(" C");
          lcd.setCursor(0, 1);
          lcd.print("<                  >");
          lcd.setCursor(0, 2);
          lcd.print("  Ventilation Fan: ");
          lcd.setCursor(7, 3);
          lcd.print("OFF");
        }
        else if (itemSelect == 0 && ventOn == true) {
          lcd.home();
          lcd.print("  Coop Temp: ");
          lcd.print(coopTemp);
          lcd.print(" C");
          lcd.setCursor(0, 1);
          lcd.print("<                  >");
          lcd.setCursor(0, 2);
          lcd.print("  Ventilation Fan: ");
          lcd.setCursor(7, 3);
          lcd.print(" ON");
        }
        if (itemSelect == 1) {
          lcd.home();
          lcd.print("  Coop Temp: ");
          lcd.print(coopTemp);
          lcd.print(" C");
          lcd.setCursor(0, 1);
          lcd.print("<                  >");
          lcd.setCursor(1, 2);
          lcd.print("Fan On Temp: ");
          lcd.print(hotTemp);
          lcd.print(" C");
        }
        break;

      case 2:
        if (itemSelect == 0 && heaterOn == false) {
          lcd.home();
          lcd.print("  Water Temp: ");
          lcd.print(waterTemp);
          lcd.print(" C");
          lcd.setCursor(0, 1);
          lcd.print("<                  >");
          lcd.setCursor(4, 2);
          lcd.print("Water Heat: ");
          lcd.setCursor(7, 3);
          lcd.print("OFF");
        }
        else if (itemSelect == 0 && heaterOn == true) {
          lcd.home();
          lcd.print("  Water Temp: ");
          lcd.print(waterTemp);
          lcd.print(" C");
          lcd.setCursor(0, 1);
          lcd.print("<                  >");
          lcd.setCursor(4, 2);
          lcd.print("Water Heat: ");
          lcd.setCursor(7, 3);
          lcd.print("ON");
        }
        if (itemSelect == 1) {
          lcd.home();
          lcd.print("  Water Temp: ");
          lcd.print(waterTemp);
          lcd.print(" C");
          lcd.setCursor(0, 1);
          lcd.print("<                  >");
          lcd.setCursor(1, 2);
          lcd.print("Heat On Temp: ");
          lcd.print(coldTemp);
          lcd.print(" C");
        }
        break;

      case 3:
        if (itemSelect == 0 && doorOpen == true) {
          lcd.home();
          lcd.print("  Coop Door: ");
          lcd.print("OPEN");
          lcd.setCursor(0, 1);
          lcd.print("<                  >");
          lcd.setCursor(3, 2);
          lcd.print("Light Value: ");
          lcd.setCursor(7, 3);
          lcd.print(photocellReading);
        }
        else if (itemSelect == 0 && doorOpen == false) {
          lcd.home();
          lcd.print("  Coop Door: ");
          lcd.print("CLOSED");
          lcd.setCursor(0, 1);
          lcd.print("<                  >");
          lcd.setCursor(3, 2);
          lcd.print("Light Value: ");
          lcd.setCursor(7, 3);
          lcd.print(photocellReading);
        }
        if (itemSelect == 1 && doorOpen == true) {
          lcd.home();
          lcd.print("  Coop Door: ");
          lcd.print("OPEN");
          lcd.setCursor(0, 1);
          lcd.print("<                  >");
          lcd.setCursor(2, 2);
          lcd.print("Light Adjust: ");
          lcd.print(closeDoor);
        }
        if (itemSelect == 1 && doorOpen == false) {
          lcd.home();
          lcd.print("  Coop Door: ");
          lcd.print("CLOSED");
          lcd.setCursor(0, 1);
          lcd.print("<                  >");
          lcd.setCursor(2, 2);
          lcd.print("Light Adjust: ");
          lcd.print(closeDoor);
        }
        break;

      case 4:
        if (itemSelect == 0 && doorOpen == true) {
          lcd.home();
          lcd.print("  Door OVERRIDE:");
          lcd.setCursor(0, 1);
          lcd.print("<     OPEN         >");
          lcd.setCursor(0, 3);
          lcd.print("Click ENTER to Close");
        }
        else if (itemSelect == 0 && doorOpen == false) {
          lcd.home();
          lcd.print("  Door OVERRIDE:");
          lcd.setCursor(0, 1);
          lcd.print("<     CLOSED       >");
          lcd.setCursor(0, 3);
          lcd.print("Click ENTER to Open");
        }
        break;

      case 5:
        if (itemSelect == 0 && layLightOn == true) {
          lcd.home();
          lcd.print("  LayLight Timer:");
          lcd.setCursor(0, 1);
          lcd.print("<      ON          >");
          lcd.setCursor(0, 3);
          lcd.print("  ENTER to turn OFF");
        }
        else if (itemSelect == 0 && layLightOn == false) {
          lcd.home();
          lcd.print("  LayLight Timer:");
          lcd.setCursor(0, 1);
          lcd.print("<     OFF         >");
          lcd.setCursor(0, 3);
          lcd.print("  ENTER to turn ON");
        }
        break;
    }

    if ((unsigned long)(millis() - lastDisplayTimer) >= displayTimer) {
      menuOn = false;
    }
  }
  else if (!menuOn) { // if the display is supposed to be off
    lcd.noDisplay(); // turn the display off
    lcd.noBacklight();
    optionSelect = 0; //back to front screen
    itemSelect = 0;
  }
  if ((buttonState1 == 0) && (!menuOn)) {
    menuOn = true;
    lastDisplayTimer = millis();
    optionSelect = 0; //back to front screen
    itemSelect = 0;
    if (nightTimer) {
      nightLightOn = true;
      lastNightLightTime = millis();
    }
  }
}



void loop() {

  readButtons();
  ventFan();
  photoCell();
  displayMenu();
  doCoopDoor();
  doorOverRide();
  waterHeat();
  layLight();
  nightLight();
}
