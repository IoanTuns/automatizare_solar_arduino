#include "SolarWebServer.h"
#include "config.h" // Include the header with function prototypes
#include "SensorData.h" // Include the shared header for sensor data
#include "DoorControl.h"
#include <Adafruit_PCF8574.h>
extern Adafruit_PCF8574 pcf;
extern DoorControl doorControl;

/**
 * @file SolarWebServer.cpp
 * @brief Implementation of the SolarWebServer class, which manages a web server for the garden automation system.
 * 
 * This file contains the implementation of the SolarWebServer class methods, which handle the initialization
 * and operation of a web server that provides real-time sensor data and system status to connected clients.
 */

/**
 * @brief Constructs a SolarWebServer object with a specified port.
 * @param port The port number on which the web server will listen. Defaults to WEB_SERVER_PORT.
 */

int status = WL_IDLE_STATUS;

SolarWebServer::SolarWebServer(uint16_t port) : _server(port) {}

/**
 * @brief Starts the web server and connects to a WiFi network.
 * @param ssid The SSID of the WiFi network to connect to.
 * @param pass The password for the WiFi network.
 * 
 * This method first connects to the specified WiFi network and then starts the server. It is useful
 * for initializing the server in environments where WiFi credentials are required.
 */
void SolarWebServer::begin(const char* ssid, const char* pass) {
    // Check for the WiFi module
    if (WiFi.status() == WL_NO_MODULE) {
        Serial.println("Communication with WiFi module failed!");
        // Don't continue
        while (true);
    }

    // Check WiFi firmware
    String fv = WiFi.firmwareVersion();
    if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
        Serial.println("Please upgrade the firmware");
    }

    // Attempt to connect to WiFi network
    while (status != WL_CONNECTED) {
        Serial.print("Attempting to connect to SSID: ");
        Serial.println(ssid);
        // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
        status = WiFi.begin(ssid, pass);

        // Wait 10 seconds for connection
        delay(10000);
    }

    _server.begin();
    // You're connected now, so print out the status
    printWifiStatus();
    Serial.println("\n[Web] Connected, IP: " + WiFi.localIP().toString());
}

/**
 * @brief Prints the current WiFi status, including SSID, IP address, and signal strength.
 */
void SolarWebServer::printWifiStatus() {
    // Print the SSID of the network you're attached to
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    // Print your board's IP address
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);

    // Print the received signal strength
    long rssi = WiFi.RSSI();
    Serial.print("Signal strength (RSSI): ");
    Serial.print(rssi);
    Serial.println(" dBm");
}

/**
 * @brief Handles incoming client requests.
 * @param tInt Interior temperature reading.
 * @param hInt Interior humidity reading.
 * @param tExt Exterior temperature reading.
 * @param hExt Exterior humidity reading.
 * 
 * This method processes client requests and sends an HTML page with sensor data. It checks for
 * new client connections, reads incoming data, and responds with a dynamically generated web page
 * displaying the current sensor readings and system information.
 */
void SolarWebServer::handleClient(float tInt, float hInt, float tExt, float hExt, const int* soilMoisture, const String& rtcTime) {
    WiFiClient client = _server.available();
    if (!client) return;

    // Read the request
    String req = client.readStringUntil('\r');
    client.flush();

    // Process commands
    if      (req.indexOf("/pump1/on")  > 0) pcf.digitalWrite(PCF_PUMP1_PIN, LOW);
    else if (req.indexOf("/pump1/off") > 0) pcf.digitalWrite(PCF_PUMP1_PIN, HIGH);
    if      (req.indexOf("/pump2/on")  > 0) pcf.digitalWrite(PCF_PUMP2_PIN, LOW);
    else if (req.indexOf("/pump2/off") > 0) pcf.digitalWrite(PCF_PUMP2_PIN, HIGH);
    if      (req.indexOf("/pump3/on")  > 0) pcf.digitalWrite(PCF_PUMP3_PIN, LOW);
    else if (req.indexOf("/pump3/off") > 0) pcf.digitalWrite(PCF_PUMP3_PIN, HIGH);

    if      (req.indexOf("/fan/on")    > 0) pcf.digitalWrite(PCF_FAN1_PIN, LOW);
    else if (req.indexOf("/fan/off")   > 0) pcf.digitalWrite(PCF_FAN1_PIN, HIGH);

    // Trap controls
    if      (req.indexOf("/trap/open") > 0) openTrap();
    else if (req.indexOf("/trap/close")> 0) closeTrap();
    else if (req.indexOf("/trap/up")   > 0) trapUp();
    else if (req.indexOf("/trap/down") > 0) trapDown();
    else if (req.indexOf("/trap/stop") > 0) stopTrap();

    // Individual door controls
    if      (req.indexOf("/door/open") > 0) doorControl.open(0);
    else if (req.indexOf("/door/close")> 0) doorControl.close(0);
    else if (req.indexOf("/door/up") > 0) doorControl.up(0);
    else if (req.indexOf("/door/down") > 0) doorControl.down(0);
    else if (req.indexOf("/door/stop") > 0) doorControl.stop(0);

    // Send HTML response
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html; charset=UTF-8");
    client.println();
    client.println(R"rawliteral(
        <!DOCTYPE html>
        <html lang="en">
        <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>Solar Control Panel</title>
        <link rel="stylesheet" href="https://fonts.googleapis.com/css?family=Roboto:400,700&display=swap">
        <style>
            body {
            font-family: 'Roboto', Arial, sans-serif;
            margin: 0; padding: 0;
            background: #e9f5ff;
            color: #222;
            }
            .container {
            max-width: 900px;
            margin: auto;
            padding: 16px;
            }
            h1 {
            text-align: center;
            color: #007BFF;
            margin-bottom: 16px;
            }
            .grid {
            display: flex;
            flex-wrap: wrap;
            gap: 16px;
            justify-content: center;
            }
            .card {
            background: #fff;
            border-radius: 12px;
            box-shadow: 0 2px 8px rgba(0,0,0,0.08);
            padding: 20px;
            flex: 1 1 260px;
            min-width: 260px;
            max-width: 350px;
            margin-bottom: 16px;
            }
            .card h2 {
            margin-top: 0;
            font-size: 1.2em;
            color: #007BFF;
            }
            .status {
            font-weight: bold;
            font-size: 1.1em;
            margin: 4px 0;
            }
            .status.ok { color: #28a745; }
            .status.warn { color: #ffc107; }
            .status.err { color: #dc3545; }
            .btn {
            display: inline-block;
            margin: 6px 4px;
            padding: 12px 20px;
            font-size: 1em;
            border: none;
            border-radius: 6px;
            background: #007BFF;
            color: #fff;
            text-decoration: none;
            transition: background 0.2s;
            box-shadow: 0 1px 3px rgba(0,0,0,0.07);
            cursor: pointer;
            }
            .btn.off { background: #dc3545; }
            .btn:active { background: #0056b3; }
            .btn.up { background: #28a745; }
            .btn.up:active { background: #1e7e34; }
            .btn.down { background: #dc3545; }
            .btn.down:active { background: #c82333; }
            .btn.stop { background: #6c757d; }
            .btn.stop:active { background: #495057; }
            @media (max-width: 600px) {
                .grid { flex-direction: column; gap: 0; }
                .card { margin-bottom: 20px; }
                .btn { min-width: 110px; width: auto; margin: 8px 4px; }
            }
            .icon { font-size: 1.1em; margin-right: 3px; vertical-align: middle; }
            .row-flex {
                display: flex;
                gap: 12px;
                justify-content: space-between;
                align-items: flex-start;
            }
            @media (max-width: 700px) {
                .row-flex { flex-direction: column; gap: 0; }
            }
        </style>
        </head>
        <body>
        <div class="container">
            <h1>☀️ Solar Control Panel</h1>  
            <div><b>RTC Time:</b> )rawliteral"); client.print(rtcTime); client.println("</div>");
            client.println(R"rawliteral(
            <div class="grid">
            <div class="card">
                <h2><span class="icon">🌡️</span>Climate and Humidity</h2>
                <div class="status ok"><b>Temperature Interior:</b> )rawliteral"); client.print(tInt, 1); client.println(" &deg;C</div>");
                client.println(R"rawliteral(
                        <div class="status ok"><b>Temperature Exterior:</b> )rawliteral"); client.print(tExt, 1); client.println(" &deg;C</div>");
                client.println(R"rawliteral(
                        <div class="status ok"><b>Humidity Interior:</b> )rawliteral"); client.print(hInt, 1); client.println(" %</div>");
                client.println(R"rawliteral(
                        <div class="status ok"><b>Humidity Exterior:</b> )rawliteral"); client.print(hExt, 1); client.println(" %</div>");
                client.println(R"rawliteral(
            </div>
            <div class="card">
                <h2><span class="icon">🌱</span>Soil Moisture</h2>
                <div class="status"><b>Soil 1:</b> )rawliteral"); client.print(soilStatus[0]); client.println(" ("); client.print(soilMoisture[0]); client.println(")</div>");
    client.println(R"rawliteral(
                <div class="status"><b>Soil 2:</b> )rawliteral"); client.print(soilStatus[1]); client.println(" ("); client.print(soilMoisture[1]); client.println(")</div>");
    client.println(R"rawliteral(
                <div class="status"><b>Soil 3:</b> )rawliteral"); client.print(soilStatus[2]); client.println(" ("); client.print(soilMoisture[2]); client.println(")</div>");
    client.println(R"rawliteral(
            </div>

            <div class="card">
                <h2><span class="icon">💧</span>Irrigation Pumps</h2>
                <div class="row-flex">
                    <div style="flex:1; min-width:150px;">
                        <h3><span class="icon">🚿</span>Valves</h3>
                        <div><b>Valve 1:</b> )rawliteral"); client.print(valveStatus[0]); client.println("</div>");
                        client.println(R"rawliteral(
                                        <div><b>Valve 2:</b> )rawliteral"); client.print(valveStatus[1]); client.println("</div>");
                        client.println(R"rawliteral(
                                        <div><b>Valve 3:</b> )rawliteral"); client.print(valveStatus[2]); client.println("</div>");
                        client.println(R"rawliteral(
                    </div>
                    <div style="flex:1; min-width:150px;">
                        <h3><span class="icon">🛢️</span>Pumps Status</h3>
                        <div><b>Pump 1:</b> )rawliteral"); client.print(pumpStatus[0]); client.println("</div>");
                        client.println(R"rawliteral(
                                        <div><b>Pump 2:</b> )rawliteral"); client.print(pumpStatus[1]); client.println("</div>");
                        client.println(R"rawliteral(
                                        <div><b>Pump 3:</b> )rawliteral"); client.print(pumpStatus[2]); client.println("</div>");
                        client.println(R"rawliteral(
                    </div>
                </div>
                <a href="/pump1/on"  class="btn">Pump 1 ON</a>
                <a href="/pump1/off" class="btn off">Pump 1 OFF</a><br>
                <a href="/pump2/on"  class="btn">Pump 2 ON</a>
                <a href="/pump2/off" class="btn off">Pump 2 OFF</a><br>
                <a href="/pump3/on"  class="btn">Pump 3 ON</a>
                <a href="/pump3/off" class="btn off">Pump 3 OFF</a>
            </div>

            <div class="card">
                <h2><span class="icon">🌀</span>Fans</h2>
                <div class="status"><b>Fan 1:</b> )rawliteral"); client.print(fanStatus[0]); client.println("</div>");
    client.println(R"rawliteral(
                <div class="status"><b>Fan 2:</b> )rawliteral"); client.print(fanStatus[1]); client.println("</div>");
    client.println(R"rawliteral(
                <a href="/fan/on"  class="btn">Fan ON</a>
                <a href="/fan/off" class="btn off">Fan OFF</a>
            </div>

            <div class="card">
                <h2><span class="icon">🪟</span>Ventilation Trap</h2>
                <div class="status"><b>Trap:</b> )rawliteral"); client.print(trapStatus); client.println("</div>");
    client.println(R"rawliteral(
                <a href="/trap/up"    class="btn up">Up</a>
                <a href="/trap/stop"  class="btn stop">Stop</a>
                <a href="/trap/down"  class="btn down">Down</a>
            </div>

            <div class="card">
                <h2><span class="icon">🚪</span>Doors</h2>
                <div class="status"><b>Door 1:</b> )rawliteral"); client.print(doorStatus[0]); client.println("</div>");
    client.println(R"rawliteral(
                <div class="status"><b>Door 2:</b> )rawliteral"); client.print(doorStatus[1]); client.println("</div>");
    client.println(R"rawliteral(
                <a href="/door/open"  class="btn">Open Door</a>
                <a href="/door/close" class="btn off">Close Door</a>
                <div>
                    <a href="/door/up"    class="btn up">Up</a>
                    <a href="/door/stop"  class="btn stop">Stop</a>
                    <a href="/door/down"  class="btn down">Down</a>
                </div>
            </div>

            <div class="card">
                <h2><span class="icon">🌧️</span>Rain</h2>
                <div class="status"><b>Rain:</b> )rawliteral"); client.print(rainStatus); client.println("</div>");
    client.println(R"rawliteral(        </div>

        </div>
        <div style="text-align:center; margin-top:24px; color:#888;">
            <small>Mobile-friendly &copy; Solar Automation</small>
        </div>
    </div>
    </body>
    </html>
    )rawliteral");
    client.stop();
}