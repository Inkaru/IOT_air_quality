#include "Arduino.h"
#include <TH02_dev.h>
#include <TinyGPS++.h>
#include "WiFi.h"

// ================= WIFI var =================
WiFiClient client;
const IPAddress server(192,168,169,1);  // sur windows, faire ipconfig et prendre l'ip de la connexion partagée
const int httpPort = 8080;
const char* ssid = "Connectify-Test";
const char* password =  "bozowoc5";

// ================= GPS var =================

static const uint32_t GPSBaud = 9600;
// The TinyGPS++ object
TinyGPSPlus gps;
// The serial connection to the GPS device
HardwareSerial ss(2);

void setup()
{
  Serial.begin(115200);

  // ================= WIFI setup =================
  Serial.println();
  Serial.println("Booted");
  Serial.println("Connecting to Wi-Fi");

  WiFi.begin (ssid, password);
  WiFi.mode(WIFI_STA);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    yield();
  }
  Serial.println("WiFi connected");

  // ================= THO2 setup =================
  delay(150);
  TH02.begin();
  delay(100);
  Serial.println("TH02_dev is available.\n");   

  // ================= GPS setup =================
  ss.begin(GPSBaud);
}

// The loop function is called in an endless loop
void loop()
{
  // read THO2
  float temper = TH02.ReadTemperature();
  float humidity = TH02.ReadHumidity();
  Serial.println("Température : ");
  Serial.println(temper);
  Serial.println("Humidité : ");
  Serial.println(humidity);

  // read GPS
  Serial.println("Latitude : ");
  float lat = gps.location.lat();
  Serial.println("Longitude : ");
  float lng = gps.location.lng();
  
  TinyGPSDate d = gps.date;
  TinyGPSTime t = gps.time;
  char date[32];
  sprintf(date, "%02d/%02d/%02d ", d.month(), d.day(), d.year());
  char hour[32];
  sprintf(hour, "%02d:%02d:%02d ", t.hour(), t.minute(), t.second());
  Serial.println("Date : ");
  Serial.println(date);
  Serial.println("Time : ");
  Serial.println(hour);
  
  // Connection to server
    if (!client.connect(server, httpPort)) {
        Serial.println("Connection failed.");
        Serial.println("Waiting 5 seconds before retrying...");
        delay(5000);
        return;
    }

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
    delay(5000);
}
