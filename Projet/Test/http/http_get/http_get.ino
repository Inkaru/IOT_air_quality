#include "Arduino.h"

#include "WiFi.h"



WiFiClient client;
const IPAddress server(192,168,169,1);  // sur windows, faire ipconfig et prendre l'ip de la connexion partagée

const int httpPort = 8080;


const char* ssid = "Connectify-Test";
const char* password =  "bozowoc5";

void setup()
{
  Serial.begin(115200);
  
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




}

// The loop function is called in an endless loop
void loop()
{
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
