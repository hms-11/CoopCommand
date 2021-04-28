
#define BLYNK_PRINT Serial
#define BUTTON 13
#define LED 4
#define DOORUP 14
#define DOORDOWN 15

#include "esp_camera.h"
#include "SPI.h"
#include "driver/rtc_io.h"
#include "ESP32_MailClient.h"
#include <FS.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <ArduinoOTA.h>
WidgetLED led1(V1);
WidgetLED led2(V2);
WidgetLED led3(V3);
WidgetLED led4(V4);


// Your WiFi credentials & Authentication Code from Blynk
// Set password to "" for open networks.
const char* ssid = "BELL101";
const char* password = "425D5FADAAEF";
char auth[] = "TTfMy-TdtJ0e_P23HuxVRrO3gqJ5d8Kl";  //sent by Blynk

// Serial Communication Variables

char coopRx; // Info received from CoopCommand
bool newDataRx = false; //has CoopCam received new data from CoopCommand
bool takePhoto = false; //Did the Blynk App request a photo?

// Timers

unsigned long photoTimer = 500; // Time to run flash before taking photo
unsigned long lastPhotoTimer = 0; // Last time the flash timer was checked
unsigned long wifiTimer = 30000; // How often to try re-connecting if WIFI lost
unsigned long lastWifiTimer = 0; // Last time the WIFI re-connect timer was checked



// To send Email using Gmail use port 465 (SSL) and SMTP Server smtp.gmail.com
// YOU MUST ENABLE less secure app option https://myaccount.google.com/lesssecureapps?pli=1
#define emailSenderAccount    "chickencammassrd@gmail.com"
#define emailSenderPassword   "Coldbeer84"
#define smtpServer            "smtp.gmail.com"
#define smtpServerPort        465
#define emailSubject          "Chicken Cam Picture!"
#define emailRecipient        "searl61@gmail.com"

#define CAMERA_MODEL_AI_THINKER

#if defined(CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
#else
#error "Camera model not selected"
#endif

// The Email Sending data object contains config and data to send
SMTPData smtpData;

// Photo File Name to save in SPIFFS
#define FILE_PHOTO "/photo.jpg"

void setup() {

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  pinMode(LED, OUTPUT);
  // Serial, WI-FI & Blynk Communication
  Serial.begin(115200);
  Blynk.begin(auth, ssid, password);
  ArduinoOTA.begin();

  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 80);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);


  if (!SPIFFS.begin(true)) {
    ESP.restart();
  }
  else {
    delay(500);
  }
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // Initialize camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    return;
  }

}

// Check if photo capture was successful
bool checkPhoto( fs::FS &fs ) {
  File f_pic = fs.open( FILE_PHOTO );
  unsigned int pic_sz = f_pic.size();
  return ( pic_sz > 100 );
}

// Capture Photo and Save it to SPIFFS
void capturePhotoSaveSpiffs( void ) {
  camera_fb_t * fb = NULL; // pointer
  bool ok = 0; // Boolean indicating if the picture has been taken correctly

  do {
    // Take a photo with the camera
    fb = esp_camera_fb_get();
    if (!fb) {
      return;
    }

    // Photo file name
    File file = SPIFFS.open(FILE_PHOTO, FILE_WRITE);

    // Insert the data in the photo file
    if (!file) {
    }
    else {
      file.write(fb->buf, fb->len); // payload (image), payload length

    }
    // Close the file
    file.close();
    esp_camera_fb_return(fb);

    // check if file has been correctly saved in SPIFFS
    ok = checkPhoto(SPIFFS);
  } while ( !ok );
}

void sendPhoto( void ) {
  // Preparing email
  // Set the SMTP Server Email host, port, account and password
  smtpData.setLogin(smtpServer, smtpServerPort, emailSenderAccount, emailSenderPassword);

  // Set the sender name and Email
  smtpData.setSender("ESP32-CAM", emailSenderAccount);

  // Set Email priority or importance High, Normal, Low or 1 to 5 (1 is highest)
  smtpData.setPriority("High");

  // Set the subject
  smtpData.setSubject(emailSubject);

  // Set the email message in HTML format
  smtpData.setMessage("<h2>Photo captured with ESP32-CAM and attached in this email.</h2>", true);
  // Set the email message in text format
  //smtpData.setMessage("Photo captured with ESP32-CAM and attached in this email.", false);

  // Add recipients, can add more than one recipient
  smtpData.addRecipient(emailRecipient);
  //smtpData.addRecipient(emailRecipient2);

  // Add attach files from SPIFFS
  smtpData.addAttachFile(FILE_PHOTO, "image/jpg");
  // Set the storage type to attach files in your email (SPIFFS)
  smtpData.setFileStorageType(MailClientStorageType::SPIFFS);

  smtpData.setSendCallback(sendCallback);

  // Start sending Email, can be set callback function to track the status
  if (!MailClient.sendMail(smtpData))

    // Clear all data from Email object to free memory
    smtpData.empty();
}

// Callback function to get the Email sending status
void sendCallback(SendStatus msg) {
  //Print the current status
}

void coopCom ( void ) {
  if (Serial.available() > 0) {
    coopRx = Serial.read();
    newDataRx = true;
  }
  if (newDataRx == true) {
    if (coopRx == 'O') { //If CoopCommand says the door is up
      led1.on();
      led2.off();
      led3.off();
      led4.off();
      newDataRx = false;
    }
    if (coopRx == 'S') { //If CoopCommand says the door is down
      led1.off();
      led2.on();
      led3.off();
      led4.off();
      newDataRx = false;
    }
    if (coopRx == 'U') { //If CoopCommand says the door is opening
      led1.off();
      led2.off();
      led3.on();
      led4.off();
      newDataRx = false;
    }
    if (coopRx == 'D') { //If CoopCommand says the door is closing
      led1.off();
      led2.off();
      led3.off();
      led4.on();
      newDataRx = false;
    }
  }
  if (digitalRead(DOORUP) == HIGH) {
    Serial.print('U');
  }
  if (digitalRead(DOORDOWN) == HIGH) {
    Serial.print('D');
  }
}

void photoRequest ( void ) {
  if (takePhoto) {
    digitalWrite(LED, HIGH);
    
    if ((unsigned long)(millis() - lastPhotoTimer) >= photoTimer) {
      capturePhotoSaveSpiffs();
      sendPhoto();
      digitalWrite(LED, LOW);
      takePhoto = false;
    }
  }
}

void wifiLost ( void ) {
if (WiFi.status() == WL_CONNECTED) {
  lastWifiTimer = millis();
}
else if (WiFi.status() !=WL_CONNECTED) {
   if ((unsigned long)(millis() - lastWifiTimer) >= wifiTimer) {
   ESP.restart();
  }
 }
}

void loop() {
  coopCom();
  photoRequest();
  wifiLost();
  ArduinoOTA.handle();
  Blynk.run();
  if (digitalRead(BUTTON) == HIGH) {
    takePhoto = true;
    lastPhotoTimer = millis();
  }
}
