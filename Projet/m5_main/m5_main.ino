#include <M5Stack.h>
#include <SD.h>
#include "utility/MPU9250.h"
#include <TinyGPS++.h>
#include <TH02_dev.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>

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
#define DATA_LEN 32
uint8_t Air_val[32]={0};
int16_t p_val[16]={0};
HardwareSerial air(1);

// ================= General var =================
int mode = 0;
uint32_t now;
uint32_t auto_upload;

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

void readAir() {
  int i = 0;
  while(air.available() && i < DATA_LEN){
    Air_val[i] = air.read();
    i++;
  }

  if(i < DATA_LEN){
    return;
  }

  for(int i=0,j=0;i<32;i++){
      if(i%2==0){
        p_val[j] = Air_val[i];
        p_val[j] = p_val[j] <<8;
      }else{
        p_val[j] |= Air_val[i];
        j++;
      }
  }
}

String getData() {
  // read THO2
  float temper = TH02.ReadTemperature();
  float humidity = TH02.ReadHumidity();
  // read GPS
  TinyGPSDate d = gps.date;
  TinyGPSTime t = gps.time;
  String lat = String(gps.location.lat(),8);
  String lng = String(gps.location.lng(),8);
  // read air
  readAir();
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
  donnees +=  p_val[2];
  donnees += ",";
  donnees +=  p_val[3];
  donnees += ",";
  donnees +=  p_val[4];
  donnees += ",";
  donnees +=  p_val[8];
  donnees += ",";
  donnees +=  p_val[9];
  donnees += ",";
  donnees +=  p_val[10];
  donnees += ",";
  donnees +=  p_val[11];
  donnees += ",";
  donnees +=  p_val[12];
  donnees += ",";
  donnees +=  p_val[13];
  donnees += ";";
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
      smartDelay(500);
    }
    Serial.println("Failed");
    return false;
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

void startWebServer(){
    webServerStarted = true;
    WiFi.disconnect();
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(apSSID);
    WiFi.mode(WIFI_MODE_AP);
    
    Serial.print("Starting Web Server at ");
    Serial.println(WiFi.softAPIP());
    server.on("/data", []() {
        readAir();
        double lat = gps.location.lat();
        double lng = gps.location.lng();
        TinyGPSDate d = gps.date;
        TinyGPSTime t = gps.time;
        String s = "<h1>Live Data</h1>";
        s += "<div>Date : ";
        s += d.day();
        s += "-";
        s += d.month();
        s += "-";
        s += d.year();
        s += " ";
        s += t.hour();
        s += "h ";
        s += t.minute();
        s += "m ";
        s += t.second();
        s += "s</div>";
        s += "</br>" ;    
        s += "<div>Coordinates : ";
        s += lat;
        s += " ";
        s += lng;
        s += "</div>";
        s += "<div>Temp/Hum : ";
        s += TH02.ReadTemperature();
        s += " degrees, ";
        s += TH02.ReadHumidity();
        s += "%</div>";
        s += "</br>";
        s += "<div>";
        s += "PM1.0 : ";
        s +=  p_val[2];
        s += "</br>";
        s += "PM2.5 : ";
        s +=  p_val[3];
        s += "</br>";
        s += "PM10 : ";
        s +=  p_val[4];
        s += "</div>";
        s += "<div>";
        s += "</br>"; 
        s += "Number of Particles :";
        s += "</br>";
        s += "0.3um : ";
        s +=  p_val[8];
        s += ", 0.5um : ";
        s +=  p_val[9];
        s += ", 1.0um : ";
        s +=  p_val[10];
        s += "</br> 2.5um : ";
        s +=  p_val[11];
        s += ", 5.0um : ";
        s +=  p_val[12];
        s += ", 10um : ";
        s +=  p_val[13];
        s += "</div>";
        s += "<p><a href=\"/\">Back to menu</a></p>";
        server.send(200, "text/html", makePage("Live Data", s));
        smartDelay(1000);
    });
    server.on("/download", [] () {
      File myFile = SD.open("/data.csv");
      
      if(myFile.size() > 0){
        server.setContentLength(CONTENT_LENGTH_UNKNOWN);
        server.send(200, "text/csv", "");
        
        String thisData = "";
        while(myFile.available()){
          String ch = myFile.readStringUntil(';'); 
          Serial.println(ch);
          thisData += ch;
          thisData += "\n";
        }      
        myFile.close();
        server.sendContent(thisData);
        SD.remove("/data.csv");
      } else {
        String s = "<h1><center>No data available</center></h1> <p>Wait a little bit before retrying.</p><p><a href=\"/\">Back to menu</a></p>";
        server.send(200, "text/html", makePage("Air Quality IoT", s));
      }
    });
    
    server.onNotFound([]() {
      String s = "<h1><center>Welcome !</center></h1><p>From this page you can see our M5Stack collecting data by following \"Live data\" or downloading recent data with \"Download data\".</p><p><center><a href=\"/data\">Live Data</a></center></p><p><center><a href=\"/download\">Download data</a></center></p>";
      server.send(200, "text/html", makePage("Air Quality IoT", s));
    });

    Serial.print("Starting Access Point at \"");
    Serial.print(apSSID);
    Serial.println("\"");

    server.begin();
}

void sendData4(){

  File myFile = SD.open("/data.csv");
  String fileName = myFile.name();
  String fileSize = String(myFile.size());

  Serial.println();
  Serial.println("file exists");
  Serial.println(myFile);

  if(myFile && checkConnection()){
  if (client.connect(ip, httpPort)) {

  String thisData = "";
  while(myFile.available()){
      String ch = myFile.readStringUntil(';'); 
      Serial.println(ch);
      thisData += ch;
      thisData += "\n";
  }
  myFile.close();

//    String thisData = "2019-11-15 04:00:48,47.5207880216282,-1.5078448366876,14.9059367971057,62\n";
//    thisData += "2019-11-15 04:00:48,47.5207880216282,-1.5078448366876,14.9059367971057,62\n";
    
    client.println("POST /csv HTTP/1.1");
    client.print("Host: ");  
    client.println(WiFi.localIP());
    client.println("Connection: close\r\nContent-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");  
    client.print(thisData.length());
    client.println("\r\n");
    client.println(thisData);
    client.println("\r\n\r\n");
    Serial.println("POSTED");
    client.stop();

    SD.remove("/data.csv");
    
    
  }
  }
  mode = 0;
  WiFi.disconnect();
}

static void smartDelay(unsigned long ms){
  unsigned long start = millis();
  do 
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

void setup() {  
  // initialize the M5Stack object
    M5.begin();

    M5.Lcd.writecommand(ILI9341_DISPOFF);
    M5.Lcd.setBrightness(0);
  
    // ================= THO2 setup =================
    smartDelay(150);
    TH02.begin();
    smartDelay(100);
    Serial.println("TH02_dev is available.\n");   

    // ================= WIFI setup =================
    Serial.println();
    Serial.println("Booted");
    WiFi.mode(WIFI_STA);

      // ================= Air Quality setup =================
    air.begin(9600, SERIAL_8N1, 3, 1);
    pinMode(5, OUTPUT);
    digitalWrite(5, 1);
    smartDelay(100);
    
     // ================= GPS setup =================
    ss.begin(GPSBaud);

    // ================= General setup =================
    now = millis();
    auto_upload = millis();

}

void loop() {
  M5.update();
  buttons();

  if(millis() - auto_upload > 300000 && mode == 0) {   
    sendData4();
    auto_upload = millis();
  }

   if(millis() - now > 10000 && mode == 0){
     now = millis();
     appendFile(SD,"/data.csv",getData());
//     Serial.println(getData());
   }
   
   if(mode == 1){  // Client mode
      sendData4();
   }

   if(mode==2) {    // Server mode
        if(!webServerStarted){
          startWebServer();
        }
        
        server.handleClient();
    }  

    smartDelay(0);
}
