#include <M5Stack.h>
#include <TH02_dev.h>
#include <Wire.h>
#include <WiFi.h>
#include "esp_wifi.h"
#include "AsyncUDP.h"
#define NBSENSOR_MAX 11

const char * ssid = "M5stack";
const char * password = "12345678";

typedef struct{
  float temp;
  float hum;
  uint8_t mac[6];
} sensor_t;

sensor_t sensors[NBSENSOR_MAX];
int nbsensor = 1;

void printLCD();

uint32_t now;

AsyncUDP udp;

void setup() {
  M5.begin();
  now = millis();

  Serial.begin(115200);
  delay(10);

  //==================== TH02 ===================
  delay(150);
  /* Reset HP20x_dev */
  TH02.begin();
  delay(100);
  
  /* Determine TH02_dev is available or not */
  Serial.println("TH02_dev is available.\n");    

}

void loop() {
  if(millis() - now > 1000){
    now = millis();

   float temper = TH02.ReadTemperature(); 
   sensors[0].temp = temper;
   float humidity = TH02.ReadHumidity();
   sensors[0].hum = humidity;

    if(M5.BtnA.wasPressed()) {
      Serial.println("Pwet");
      printLCD();
    }
  }
}



void printLCD(){
  char data[50];
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.drawLine(0, 0, 0, 239, TFT_RED);
  M5.Lcd.drawLine(80, 0, 80, 239, TFT_RED);
  M5.Lcd.drawLine(160, 0, 160, 239, TFT_RED);
  M5.Lcd.drawLine(240, 0, 240, 239, TFT_RED);
  M5.Lcd.drawLine(319, 0, 319, 239, TFT_RED);
  M5.Lcd.drawLine(0, 0, 319, 0, TFT_RED);
  M5.Lcd.drawLine(0, 80, 319, 80, TFT_RED);
  M5.Lcd.drawLine(0, 160, 319, 160, TFT_RED);
  M5.Lcd.drawLine(0, 239, 319, 239, TFT_RED);
  for (int y = 0, i = 0  ; y < 3 && i < nbsensor ; y++){
    for (int x = 0; x < 4 && i < nbsensor ; x++, i++){ 
      M5.Lcd.setTextDatum(MC_DATUM);
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.setTextSize(2);
      sprintf(data, "%2.1f", sensors[i].temp);
      M5.Lcd.drawString(data, 40 + 80 * x, 20 + 80 * y, 1);
      sprintf(data, "%2.1f", sensors[i].hum);
      M5.Lcd.drawString(data, 40 + 80 * x, 40 + 80 * y, 1);
      sprintf(data, "%02x%02x%02x%02x%02x%02x", sensors[i].mac[0], sensors[i].mac[1], sensors[i].mac[2], sensors[i].mac[3], sensors[i].mac[4], sensors[i].mac[5]);
      M5.Lcd.setTextSize(1);
      M5.Lcd.drawString(data, 40 + 80 * x, 60 + 80 * y, 1);
     }
  }
}

