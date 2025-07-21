#ifndef SOLAR_WEBSERVER_H
#define SOLAR_WEBSERVER_H

#include <WiFiS3.h>
#include <DHT.h>
#include "config.h" 

class SolarWebServer {
  public:
    SolarWebServer(uint16_t port = WEB_SERVER_PORT) : _server(port) {}

    void begin() {
      _server.begin();
      Serial.print("Web server started on port ");
      Serial.println(WEB_SERVER_PORT);
    }

    void begin(const char* ssid, const char* pass) {
      begin();
    }

    void handleClient(float tInt, float hInt, float tExt, float hExt) {
      WiFiClient client = _server.available();
      
      if (client) {
        Serial.println("New client connected");
        String currentLine = "";
        
        while (client.connected()) {
          if (client.available()) {
            char c = client.read();
            if (c == '\n') {
              if (currentLine.length() == 0) {
                // Send HTTP response headers
                client.println("HTTP/1.1 200 OK");
                client.println("Content-type:text/html; charset=UTF-8");
                client.println("Connection: close");
                client.println();
                
                // Send HTML page
                client.println("<!DOCTYPE html>");
                client.println("<html><head>");
                client.println("<meta charset='UTF-8'>");
                client.println("<title>Garden Automation System</title>");
                client.println("<meta http-equiv='refresh' content='30'>");
                client.println("<style>");
                client.println("body { font-family: Arial, sans-serif; margin: 40px; background-color: #f0f8ff; }");
                client.println("h1 { color: #2e8b57; text-align: center; }");
                client.println(".sensor { background: white; padding: 20px; margin: 15px 0; border-radius: 10px; box-shadow: 0 4px 8px rgba(0,0,0,0.1); }");
                client.println(".error { background: #ffe6e6; border-left: 5px solid #ff0000; color: #cc0000; }");
                client.println(".success { color: #2e8b57; }");
                client.println("h2 { margin-top: 0; color: #333; }");
                client.println("p { font-size: 16px; margin: 10px 0; }");
                client.println("strong { color: #2e8b57; }");
                client.println("</style>");
                client.println("</head><body>");
                
                client.println("<h1>Garden Automation System</h1>");
                
                client.println("<div class='sensor'>");
                client.println("<h2>Interior Climate</h2>");
                if (!isnan(tInt) && !isnan(hInt)) {
                  client.print("<p class='success'>Temperature: <strong>");
                  client.print(tInt);
                  client.println("&deg;C</strong></p>");
                  client.print("<p class='success'>Humidity: <strong>");
                  client.print(hInt);
                  client.println("%</strong></p>");
                } else {
                  client.println("<p class='error'>ERROR: Interior sensor not responding</p>");
                }
                client.println("</div>");
                
                client.println("<div class='sensor'>");
                client.println("<h2>Exterior Climate</h2>");
                if (!isnan(tExt) && !isnan(hExt)) {
                  client.print("<p class='success'>Temperature: <strong>");
                  client.print(tExt);
                  client.println("&deg;C</strong></p>");
                  client.print("<p class='success'>Humidity: <strong>");
                  client.print(hExt);
                  client.println("%</strong></p>");
                } else {
                  client.println("<p class='error'>ERROR: Exterior sensor not responding</p>");
                }
                client.println("</div>");
                
                client.println("<div class='sensor'>");
                client.println("<h2>System Information</h2>");
                client.print("<p>IP Address: <strong>");
                client.print(WiFi.localIP());
                client.println("</strong></p>");
                client.print("<p>WiFi Network: <strong>");
                client.print(WiFi.SSID());
                client.println("</strong></p>");
                client.print("<p>Signal Strength: <strong>");
                client.print(WiFi.RSSI());
                client.println(" dBm</strong></p>");
                client.print("<p>Uptime: <strong>");
                client.print(millis() / 1000);
                client.println(" seconds</strong></p>");
                client.println("</div>");
                
                client.println("<div class='sensor'>");
                client.println("<h2>Sensor Status</h2>");
                if (!isnan(tInt) && !isnan(hInt) && !isnan(tExt) && !isnan(hExt)) {
                  client.println("<p class='success'>All sensors operational</p>");
                } else {
                  client.println("<p class='error'>Some sensors are not responding</p>");
                  client.println("<p>Check DHT22 sensor connections on pins 2 and 3</p>");
                }
                client.println("</div>");
                
                client.println("</body></html>");
                break;
              } else {
                currentLine = "";
              }
            } else if (c != '\r') {
              currentLine += c;
            }
          }
        }
        
        client.stop();
        Serial.println("Client disconnected");
      }
    }

  private:
    WiFiServer _server;
};

#endif // SOLAR_WEBSERVER_H
