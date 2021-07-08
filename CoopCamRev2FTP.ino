
#define PHOTOCLICK V5
#define LED 4
#define DOORUP V7
#define DOORDOWN V6
#define BLYNK_HEARTBEAT 30


#include "esp_camera.h"
#include "Arduino.h"
#include <WiFi.h>
#include <WiFiClient.h> 
#include <ESP32_FTPClient.h>
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include <BlynkSimpleEsp32.h>
WidgetLED led1(V1);
WidgetLED led2(V2);
WidgetLED led3(V3);
WidgetLED led4(V4);


#define WIFI_SSID "xxxx"
#define WIFI_PASS "xxxx"
char auth[] = "xxxx";  //sent by Blynk 1.0


// Pin definition for CAMERA_MODEL_AI_THINKER
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


char ftp_server[] = "files.000webhost.com";
char ftp_user[] = "xxxx";
char ftp_pass[] ="xxxx";

bool takePhoto = false; 
bool loading = false;
String displayImage = "https://coopcommandimages.000webhostapp.com/uploads/1.png";
String coopImageURL = "https://coopcommandimages.000webhostapp.com/uploads/1.png";
String coopImage = "coopPic.jpg";
int i = 1;
int ic = 1;
int imageFlip = 1;
char coopRx; // Info received from CoopCommand
bool newDataRx = false; //has CoopCam received new data from CoopCommand


BlynkTimer timer;
// WidgetLCD lcd(V1);

// you can pass a FTP timeout and debbug mode on the last 2 arguments
ESP32_FTPClient ftp (ftp_server,ftp_user,ftp_pass, 5000, 2);

void setup()
{
 pinMode(LED, OUTPUT);
 timer.setInterval(1000,coopCom);
 timer.setInterval(250,loadingImage);
  Blynk.setProperty(V0, "url", 1, coopImageURL);
  Blynk.virtualWrite(V0, 1); 
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  
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
  
  if(psramFound()){
    config.frame_size = FRAMESIZE_SVGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    config.jpeg_quality = 20;
    config.fb_count = 2;
  } 
  else {
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 20;
    config.fb_count = 1;
  }
  
  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
//    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  
  Serial.begin( 115200 );

// WiFi.begin( WIFI_SSID, WIFI_PASS );
  Blynk.begin(auth, WIFI_SSID, WIFI_PASS);
  // Serial.println("Connecting Wifi...");
 // while (WiFi.status() != WL_CONNECTED) {
 //     delay(500);
 //     Serial.print(".");
//  }
//  Serial.println("");
//  Serial.print("IP address: ");
//  Serial.println(WiFi.localIP());
}

BLYNK_CONNECTED() {
    Blynk.syncAll();
}



void loadingImage( void ) {
      if (loading) {
      displayImage= "https://coopcommandimages.000webhostapp.com/uploads/";
      displayImage+= i;
      displayImage+= ".png";
      Blynk.setProperty(V0, "url", 1, displayImage); 
      if (i <= 6) {
      i ++;
      }
      else if (i > 6) {
        i = 1;
      }
      }
      else if (!loading) {
        if (ic == 1) {
        coopImageURL = "https://coopcommandimages.000webhostapp.com/uploads/coopPic2.jpg";
        }
        else if (ic == 2) {
        coopImageURL = "https://coopcommandimages.000webhostapp.com/uploads/coopPic3.jpg";
        }
        else if (ic == 3) {
          coopImageURL = "https://coopcommandimages.000webhostapp.com/uploads/coopPic4.jpg";
        }
        Blynk.setProperty(V0, "url", 1, coopImageURL);
        Blynk.virtualWrite(V0, 1);  
        i = 1;      
      }

}

 
void sendPhoto ( void ) {
   if (takePhoto) {

   camera_fb_t * fb = NULL;
  
  // Take Picture with Camera
  fb = esp_camera_fb_get(); 
  digitalWrite(LED, LOW); 
  delay (50);
  if(!fb) {
//    Serial.println("Camera capture failed");
    ESP.restart();
    return;
  }

if (imageFlip == 1) {
  ftp.OpenConnection();
  // Create the new file and send the image
  ftp.ChangeWorkDir("/public_html/uploads/");
  ftp.InitFile("Type I");
  ftp.NewFile("coopPic2.jpg");
  ftp.WriteData( fb->buf, fb->len );
  ftp.CloseFile();
  ftp.CloseConnection();
  takePhoto = false;
  imageFlip = 2;
  ic = 1;
  esp_camera_fb_return(fb);
    Serial.print('N');
   }
else if (imageFlip == 2) {
   ftp.OpenConnection();
  // Create the new file and send the image
  ftp.ChangeWorkDir("/public_html/uploads/");
  ftp.InitFile("Type I");
  ftp.NewFile("coopPic3.jpg");
  ftp.WriteData( fb->buf, fb->len );
  ftp.CloseFile();
  ftp.CloseConnection();
  takePhoto = false;
  imageFlip = 3;
  ic = 2;
  esp_camera_fb_return(fb);
  Serial.print('N');
   }
else if (imageFlip == 3) {
   ftp.OpenConnection();
  // Create the new file and send the image
  ftp.ChangeWorkDir("/public_html/uploads/");
  ftp.InitFile("Type I");
  ftp.NewFile("coopPic4.jpg");
  ftp.WriteData( fb->buf, fb->len );
  ftp.CloseFile();
  ftp.CloseConnection();
  takePhoto = false;
  imageFlip = 1;
  ic = 3;
  esp_camera_fb_return(fb);
  Serial.print('N');
   }
}
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
}

BLYNK_WRITE(DOORUP) {
Serial.print('U');
}

BLYNK_WRITE(DOORDOWN) {
Serial.print('D');
}

BLYNK_WRITE(V5) {
digitalWrite(LED, HIGH);
Serial.print('L'); 
takePhoto = true;
loading = true;
loadingImage();
timer.setTimeout(250, sendPhoto);
timer.setTimeout(3000, []()
{
  loading = false;
  loadingImage();
});
}

void loop()
{
  Blynk.run();
  timer.run();
}
