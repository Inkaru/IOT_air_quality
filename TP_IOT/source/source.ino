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

  //==================== WIFI ===================

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  Serial.println("MAC address: ");
  WiFi.macAddress(sensors[0].mac);
  int j;
  for( j = 0; j<6; j++){
    Serial.print(sensors[0].mac[j], HEX);
    M5.Lcd.print(sensors[0].mac[j], HEX);
  }
  Serial.println("");

  //==================== TH02 ===================
  delay(150);
  /* Reset HP20x_dev */
  TH02.begin();
  delay(100);
  
  /* Determine TH02_dev is available or not */
  Serial.println("TH02_dev is available.\n");    

  // =================== UDP ===================
  
  if(udp.listen(WiFi.localIP(), 1234)) {
    Serial.println("UDP connected");
    udp.onPacket([](AsyncUDPPacket packet) {
      Serial.print("From: ");
      Serial.print(packet.remoteIP());
      Serial.print(", Data: ");
      Serial.write(packet.data(), packet.length());
      Serial.println();

      if(packet.length() == sizeof(sensor_t)){
        char found = 0;
        int k;
        sensor_t data = *(sensor_t*)packet.data();
        for(k=0;k<nbsensor && !found; k++){
          if(data.mac[0] == sensors[k].mac[0] && data.mac[3] == sensors[k].mac[3] && data.mac[5] == sensors[k].mac[5]){
              sensors[k].temp = data.temp;
              sensors[k].hum = data.hum;
              found = 1;
              Serial.println("match");
              }
          }
  
         if(!found && nbsensor <= NBSENSOR_MAX){
            sensors[nbsensor++] = data;
          }
        }      
    });
  }
}

void loop() {
  if(millis() - now > 1000){
	  now = millis();

   float temper = TH02.ReadTemperature(); 
   sensors[0].temp = temper;
   float humidity = TH02.ReadHumidity();
   sensors[0].hum = humidity;

   udp.broadcastTo((uint8_t*)sensors,sizeof(sensor_t),1234);

   printLCD();
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

