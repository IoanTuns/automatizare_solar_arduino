#ifndef SOLAR_WEBSERVER_H
#define SOLAR_WEBSERVER_H

#include <WiFiS3.h>
#include <DHT.h>
#include "config.h" 

// declară că există două variabile globale definite în .ino
extern const int PUMP_PIN;
extern const int FAN_PIN;

int status = WL_IDLE_STATUS;
// WiFiServer server(WEB_SERVER_PORT);

class SolarWebServer {
  public:
    SolarWebServer(uint16_t port = WEB_SERVER_PORT): _server(port) {}

    void begin(const char* ssid, const char* pass) {
      // check for the WiFi module:
      if (WiFi.status() == WL_NO_MODULE) {
        Serial.println("Communication with WiFi module failed!");
        // don't continue
        while (true);
      }
      // check WiFi firmware
      String fv = WiFi.firmwareVersion();
      if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
        Serial.println("Please upgrade the firmware");
      }

      // attempt to connect to WiFi network:
      while (status != WL_CONNECTED) {
        Serial.print("Attempting to connect to SSID: ");
        Serial.println(ssid);
        // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
        status = WiFi.begin(ssid, pass);

        // wait 10 seconds for connection:
        delay(10000);
      }
      _server.begin();
      // you're connected now, so print out the status:
      printWifiStatus();
      Serial.println("\n[Web] Conectat, IP: " + WiFi.localIP().toString());
    }

    void handleClient(float tInt, float hInt, float tExt, float hExt) {
      WiFiClient client = _server.available();
      if (!client) return;

      // citeşte cererea
      String req = client.readStringUntil('\r');
      client.flush();

      // procesează comenzi
      if      (req.indexOf("/pump1/on")      > 0) digitalWrite(PUMP1_PIN, LOW);
      else if (req.indexOf("/pump1/off")     > 0) digitalWrite(PUMP1_PIN, HIGH);
      if      (req.indexOf("/pump2/on")      > 0) digitalWrite(PUMP2_PIN, LOW);
      else if (req.indexOf("/pump2/off")     > 0) digitalWrite(PUMP2_PIN, HIGH);
      if      (req.indexOf("/pump3/on")      > 0) digitalWrite(PUMP3_PIN, LOW);
      else if (req.indexOf("/pump3/off")     > 0) digitalWrite(PUMP3_PIN, HIGH);

      if      (req.indexOf("/fan/on")        > 0) digitalWrite(FAN_PIN, HIGH);
      else if (req.indexOf("/fan/off")       > 0) digitalWrite(FAN_PIN, LOW);

      if      (req.indexOf("/trap/open")     > 0) openTrap();
      else if (req.indexOf("/trap/close")    > 0) closeTrap();

      if      (req.indexOf("/door/open")     > 0) openDoor();
      else if (req.indexOf("/door/close")    > 0) closeDoor();

      // răspuns HTML
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html\n");
      client.println(R"rawliteral(
      <!DOCTYPE html>
      <html>
        <head>
          <meta charset="UTF-8">
          <title>Control Solar</title>
          <style>
            body { font-family: sans-serif; margin: 0; padding: 20px; background: #f4f4f4; }
            h1 { text-align: center; }
            .grid { display: grid; grid-template-columns: repeat(2,1fr); gap: 10px; max-width: 600px; margin: auto; }
            .card { background: white; padding: 15px; border-radius: 8px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }
            .card h2 { margin-top: 0; }
            .btn { display: inline-block; margin: 5px 2px; padding: 8px 12px; text-decoration: none; background: #007BFF; color: white; border-radius: 4px; }
            .btn.off { background: #dc3545; }
            .status { font-weight: bold; }
          </style>
        </head>
        <body>
          <h1>Panou Control Solar</h1>
          <div class="grid">
            <div class="card">
              <h2>Climat</h2>
              <p>Interior: <span class="status">)rawliteral");
      client.print(tInt,1); client.print(" °C, "); client.print(hInt,1); client.println("%</span>");
      client.println(R"rawliteral(
              Exterior: <span class="status">)rawliteral");
      client.print(tExt,1); client.print(" °C, "); client.print(hExt,1); client.println("%</span>");
      client.println(R"rawliteral(
            </div>

            <div class="card">
              <h2>Pompe Irigare</h2>
              <a href="/pump1/on"  class="btn">Pompa 1 ON</a>
              <a href="/pump1/off" class="btn off">Pompa 1 OFF</a><br>
              <a href="/pump2/on"  class="btn">Pompa 2 ON</a>
              <a href="/pump2/off" class="btn off">Pompa 2 OFF</a><br>
              <a href="/pump3/on"  class="btn">Pompa 3 ON</a>
              <a href="/pump3/off" class="btn off">Pompa 3 OFF</a>
            </div>

            <div class="card">
              <h2>Ventilator</h2>
              <a href="/fan/on"  class="btn">Ventilator ON</a>
              <a href="/fan/off" class="btn off">Ventilator OFF</a>
            </div>

            <div class="card">
              <h2>Trapă Ventilație</h2>
              <a href="/trap/open"  class="btn">Deschide Trapă</a>
              <a href="/trap/close" class="btn off">Închide Trapă</a>
            </div>

            <div class="card">
              <h2>Ușă Folie</h2>
              <a href="/door/open"  class="btn">Ridică Ușa</a>
              <a href="/door/close" class="btn off">Coboară Ușa</a>
            </div>
          </div>
        </body>
      </html>
      )rawliteral");
      client.stop();
    }

    void printWifiStatus() {
      // print the SSID of the network you're attached to:
      Serial.print("SSID: ");
      Serial.println(WiFi.SSID());

      // print your board's IP address:
      IPAddress ip = WiFi.localIP();
      Serial.print("IP Address: ");
      Serial.println(ip);

      // print the received signal strength:
      long rssi = WiFi.RSSI();
      Serial.print("signal strength (RSSI):");
      Serial.print(rssi);
      Serial.println(" dBm");
    }
  private:
    WiFiServer _server;
};

#endif // SOLAR_WEBSERVER_H
