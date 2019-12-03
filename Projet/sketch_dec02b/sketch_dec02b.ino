#include <M5Stack.h>
#include <TH02_dev.h>
#include <TinyGPS++.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <SD.h>
//#include "utility/MPU9250.h"

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

// ================= GPS var =================
static const uint32_t GPSBaud = 9600;
// The TinyGPS++ object
TinyGPSPlus gps;
// The serial connection to the GPS device
HardwareSerial ss(2);

// ================= General var =================
int mode = 0;
uint32_t now;

char * getData() {
  // read THO2
  float temper = TH02.ReadTemperature();
  float humidity = TH02.ReadHumidity();
  // read GPS
  float lat = gps.location.lat();
  float lng = gps.location.lng();
  TinyGPSDate d = gps.date;
  TinyGPSTime t = gps.time;
  // print all data
  char donnees[65];
  sprintf(donnees, "%02d-%02d-%02d %02d:%02d:%02d,%f,%f,%f,%f", d.year(), d.month(), d.day(), t.hour(), t.minute(), t.second(), lat, lng, temper, humidity);
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

// Write and read on the SD card
/*void readFile(fs::FS &fs, const char * path) {
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while(file.available()){
        int ch = file.read();
        Serial.write(ch);
    }
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);
    
    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
}



void appendFile(fs::FS &fs, const char * path, const char * message){

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

}*/


void setup()
{
  Serial.begin(115200);

  // ================= WIFI setup =================
  Serial.println();
  Serial.println("Booted");
  WiFi.mode(WIFI_STA);

  // ================= THO2 setup =================
  delay(150);
  TH02.begin();
  delay(100);
  Serial.println("TH02_dev is available.\n");   

  // ================= GPS setup =================
  ss.begin(GPSBaud);

  // ================= General setup =================
  now = millis();
}

// The loop function is called in an endless loop
void loop()
{
    M5.update();
    
    if (M5.BtnA.wasReleased()) {
      Serial.println("Mode 0");
      mode = 0;
      webServerStarted = false;
    } else if (M5.BtnB.wasReleased()) {
      Serial.println("Mode 1");
      mode = 1;
      webServerStarted = false;
    } else if (M5.BtnC.wasReleased()) {
      Serial.println("Mode 2");
      mode = 2;
    }

   if(millis() - now > 2000){
       now = millis();
       Serial.println("data : ");
       Serial.println(getData());
       //appendFile(SD,"/data.txt",getData());
       Serial.println();

       Serial.println("Mode : ");
       Serial.println(mode);
   }

    if(mode == 1 && checkConnection()){  // Client mode
        // Connection to server
        if (client.connect(ip, httpPort)) {
            // Sucessful connection
            client.print("GET /get HTTP/1.0\n\n");
            int maxloops = 0;

            //wait for the server's reply to become available
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
            }
            else
            {
                Serial.println("client.available() timed out ");
            }

                Serial.println("Closing connection.");
                client.stop();

                Serial.println("Waiting 5 seconds before restarting...");
                delay(1000);

                WiFi.disconnect();
                Serial.println("WiFi disconnected");
        } else {
            // Failed connection
            Serial.println("Connection failed.");
            Serial.println("Waiting 1 second before retrying...");
            delay(1000);
        }      

    } 
    
    if(mode==2) {    // Server mode
        if(!webServerStarted){
          startWebServer();
        }
        server.handleClient();
    }    

    
}
