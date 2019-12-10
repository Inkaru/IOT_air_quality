#include <M5Stack.h>
#include <SD.h>
#include "utility/MPU9250.h"
#include <TinyGPS++.h>
#include <TH02_dev.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "DFRobot_SHT20.h"

// ================= GPS var =================
static const uint32_t GPSBaud = 9600;
// The TinyGPS++ object
TinyGPSPlus gps;
// The serial connection to the GPS device
HardwareSerial ss(2);

// ================= WIFI var =================
WiFiClient client;
const IPAddress ip(192,168,169,1);  // sur windows, faire ipconfig et prendre l'ip de la connexion partagée
const int httpPort = 8080;
const char* ssid = "Connectify-Test";
const char* password =  "bozowoc5";

// ================= WebServer var =================
WebServer server(80);
const IPAddress apIP(192, 168, 4, 1);
const char* apSSID = "M5STACK_METEO";
boolean webServerStarted = false;

////  ================= Air Quality var =================
//DFRobot_SHT20    sht20;
//#define DATA_LEN 32
//uint8_t Air_val[32]={0};
int16_t p_val[16]={0};

// ================= General var =================
int mode = 0;
uint32_t now;

void readFile(fs::FS &fs, const char * path) {
    Serial.printf("Reading file: %s\n", path);
    M5.Lcd.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        M5.Lcd.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    M5.Lcd.print("Read from file: ");
    while(file.available()){
        int ch = file.read();
        Serial.write(ch);
        M5.Lcd.write(ch);
    }
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);
    M5.Lcd.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        M5.Lcd.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
        M5.Lcd.println("File written");
    } else {
        Serial.println("Write failed");
        M5.Lcd.println("Write failed");
    }
}

void appendFile(fs::FS &fs, const char * path, String message){

    Serial.printf("Appending to file: %s\n", path);
    File file = fs.open(path, FILE_APPEND);

    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }

    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }

    file.close();

}

void buttons() {
  if (M5.BtnA.wasReleased()) {
      Serial.println("Mode 0");
      mode = 0;
      WiFi.disconnect();
      webServerStarted = false;
    } else if (M5.BtnB.wasReleased()) {
      Serial.println("Mode 1");
      mode = 1;
      WiFi.disconnect();
      webServerStarted = false;
    } else if (M5.BtnC.wasReleased()) {
      Serial.println("Mode 2");
      mode = 2;
    }
}

//void readAir() {
//  int i = 0;
//  while(Serial2.available() && i < DATA_LEN){
//    Air_val[i] = Serial2.read();
//    i++;
//  }
//
//  if(i < DATA_LEN){
//    return;
//  }
//
//  for(int i=0,j=0;i<32;i++){
//      if(i%2==0){
//        p_val[j] = Air_val[i];
//        p_val[j] = p_val[j] <<8;
//      }else{
//        p_val[j] |= Air_val[i];
//        j++;
//      }
//  }
//}

String getData() {
  // read THO2
  float temper = TH02.ReadTemperature();
  float humidity = TH02.ReadHumidity();
  // read GPS
  float lat = gps.location.lat();
  float lng = gps.location.lng();
  TinyGPSDate d = gps.date;
  TinyGPSTime t = gps.time;
  // read air
//  readAir();
  // print all data
  String donnees;
//  sprintf(donnees, "%02d-%02d-%02d %02d:%02d:%02d,%f,%f,%f,%f,%d,%d,%d,%d,%d,%d,%d,%d,%d", d.year(), d.month(), d.day(), t.hour(), t.minute(), t.second(), lat, lng, temper, humidity,  p_val[2],  p_val[3],  p_val[4],  p_val[8],  p_val[9],  p_val[10],  p_val[11],  p_val[12],  p_val[13]);
  donnees += d.year();
  donnees += "-";
  donnees += d.month();
  donnees += "-";
  donnees += d.day();
  donnees += " ";
  donnees += t.hour();
  donnees += ":";
  donnees += t.minute();
  donnees += ":";
  donnees += t.second();
  donnees += ",";
  donnees += lat;
  donnees += ",";
  donnees += lng;
  donnees += ",";
  donnees += temper;
  donnees += ",";
  donnees += humidity;
  donnees += ",";
  donnees += p_val[2];
  donnees += ",";
  donnees += p_val[3];
  donnees += ",";
  donnees += p_val[4];  
  donnees += ",";
  donnees += p_val[8];  
  donnees += ",";
  donnees += p_val[9];  
  donnees += ",";
  donnees += p_val[10];  
  donnees += ",";
  donnees += p_val[11];  
  donnees += ",";
  donnees += p_val[12];  
  donnees += ",";
  donnees += p_val[13];
  donnees += '\n';
  return donnees;
}

boolean checkConnection() {
    WiFi.mode(WIFI_STA);
    Serial.println("Connecting to Wi-Fi");
    WiFi.begin (ssid, password);
    int wait = 0;
    while (wait < 20) {
      if(WiFi.status() == WL_CONNECTED){
        Serial.println("WiFi connected");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        return true;
      }
      wait++;
      Serial.print(".");
      delay(500);
    }
    Serial.println("Failed");
    return false;
}

void startWebServer(){
    webServerStarted = true;
    WiFi.disconnect();
    delay(100);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(apSSID);
    WiFi.mode(WIFI_MODE_AP);
    
    Serial.print("Starting Web Server at ");
    Serial.println(WiFi.softAPIP());
    server.on("/data", []() {
        String s = "<h1>Live Data</h1>";
        s += "<div>Date : ";
        s += gps.date.day();
        s += "-";
        s += gps.date.month();
        s += "-";
        s += gps.date.year();
        s += " ";
        s += gps.time.hour();
        s += "-";
        s += gps.time.minute();
        s += "-";
        s += gps.time.second();
        s += "</div>";        
        s += "<div>Coordinates : ";
        s += gps.location.lat();
        s += ", ";
        s += gps.location.lng();
        s += "</div>";
        s += "<div>";
        s += "PM1.0 :";
        s +=  p_val[2];
        s += ", PM2.5 :";
        s +=  p_val[3];
        s += ", PM10 :";
        s +=  p_val[4];
        s += "</div>";
        s += "<div>";
        s += "Number of Particles :";
        s += "</br>";
        s += "0.3um :";
        s +=  p_val[8];
        s += ", 0.5um :";
        s +=  p_val[9];
        s += ", 1.0um :";
        s +=  p_val[10];
        s += ", 2.5um :";
        s +=  p_val[11];
        s += ", 5.0um :";
        s +=  p_val[12];
        s += ", 10um :";
        s +=  p_val[13];
        s += "</div>";
        server.send(200, "text/html", makePage("Live Data", s));
    });
    server.onNotFound([]() {
      String s = "<h1>Not Found</h1><p><a href=\"/data\">Live Data</a></p>";
      server.send(200, "text/html", makePage("Not found", s));
    });

    Serial.print("Starting Access Point at \"");
    Serial.print(apSSID);
    Serial.println("\"");

    server.begin();
}

String makePage(String title, String contents) {
  String s = "<!DOCTYPE html><html><head>";
  s += "<meta name=\"viewport\" content=\"width=device-width,user-scalable=0\">";
  s += "<title>";
  s += title;
  s += "</title></head><body>";
  s += contents;
  s += "</body></html>";
  return s;
}

void sendData(){

//  File file = SD.open("/data.txt");
  
  if (client.connect(ip, httpPort)) {
      Serial.println("POST");
      // Sucessful connection
      client.println("POST /upload HTTP/1.1");
      client.print("Host: ");
      client.println(WiFi.localIP());
      client.println("Content-Type: multipart/form-data; boundary=----123");
      client.println();    
      client.println("------123");
      client.println("Content-Disposition: form-data; name=\"myFile\"; filename=\"MOCK_DATA2.csv\"");
      client.println("Content-Type: application/vnd.ms-excel");  
      client.println("");
      String data = "2019-11-15 04:00:48,47.5207880216282,-1.5078448366876,14.9059367971057,62";
      client.println(data);
      client.println("------123--\n\n");

//      client.print("POST /upload HTTP/1.0");
//      client.println("Content-Type: text/csv");
//      client.println();    
//      client.println("2019-11-15 04:00:48,47.5207880216282,-1.5078448366876,14.9059367971057,62");
      int maxloops = 10;
            while (!client.available() && maxloops < 1000)
            {
                maxloops++;
                delay(1); //delay 1 msec
            }
            if (client.available() > 0)
            {
                //read back one line from the server
                String line = client.readStringUntil('\r');
                Serial.println(line);
                if(line == "OK 200") {  //TODO : vérifier que c'est le bon message
                  SD.remove("/data.csv");
                }
            }
            else
            {
                Serial.println("client.available() timed out ");
            }
            client.stop();
        
  } else {
    Serial.println("Connection failed");
  }
}

void setup() {
  // put your setup code here, to run once:
  
  // initialize the M5Stack object
    M5.begin();
  
//    M5.Lcd.fillScreen(BLACK);
//    M5.Lcd.setCursor(0, 10);
//    M5.Lcd.printf("TF card test:\r\n");
//
//    //writeFile(SD, "/hello.txt", "Hello Pwet\n");
//    appendFile(SD,"/hello.txt","Allez");
//    readFile(SD, "/hello.txt");

     // ================= GPS setup =================
    ss.begin(GPSBaud);

    // ================= THO2 setup =================
    delay(150);
    TH02.begin();
    delay(100);
    Serial.println("TH02_dev is available.\n");   

    // ================= WIFI setup =================
    Serial.println();
    Serial.println("Booted");
    WiFi.mode(WIFI_STA);

    // ================= General setup =================
    now = millis();

}

void loop() {
  M5.update();
  buttons();

   if(millis() - now > 4000 && !webServerStarted){
     now = millis();
     String mess = "temp : ";
     mess += TH02.ReadTemperature();
     appendFile(SD,"/data.csv",getData());
     Serial.println(getData());

   }

   if(mode==2) {    // Server mode
        if(!webServerStarted){
          startWebServer();
        }
        server.handleClient();
    }  
}
