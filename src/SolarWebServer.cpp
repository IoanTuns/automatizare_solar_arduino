#include "SolarWebServer.h"
#include "config.h" // Include the header with function prototypes

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

SolarWebServer::SolarWebServer(uint16_t port, PumpControl& pumpControl, DoorControl& doorControl, ClimateControl& climateControl, ValveControl& valveControl, TrapControl& trapControl, IrrigationControl& irrigationControl)
    : _server(port),
      _pumpControl(pumpControl),
      _doorControl(doorControl),
      _valveControl(valveControl),
      _climateControl(climateControl),
      _trapControl(trapControl),
      _irrigationControl(irrigationControl) {}

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
        Serial.println("ERROR: Communication with WiFi module failed! The server will not start.");
        // Don't hang the system; just exit the function.
        // The rest of the application can run in offline mode.
        return;
    }

    // Check WiFi firmware
    String fv = WiFi.firmwareVersion();
    if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
        Serial.println("Warning: WiFi firmware is not the latest version. Please upgrade.");
    }

    // Attempt to connect to WiFi network with a 30-second timeout
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    WiFi.begin(ssid, pass);

    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - startTime < 30000)) {
        Serial.print(".");
        delay(500);
    }

    if (WiFi.status() == WL_CONNECTED) {
        _server.begin();
        printWifiStatus();
        Serial.println("\n[Web] Connected, IP: " + WiFi.localIP().toString());
    } else {
        Serial.println("\n[Web] Failed to connect to WiFi after 30 seconds. Continuing in offline mode.");
    }
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
 * @param sensors A struct containing all current sensor readings.
 * @param rtcTime A formatted string of the current time.
 * 
 * This method processes client requests and sends an HTML page with sensor data. It checks for
 * new client connections, reads incoming data, and responds with a dynamically generated web page
 * displaying the current sensor readings and system information.
 */
void SolarWebServer::handleClient(const SensorData& sensors, const String& rtcTime) {
    WiFiClient client = _server.available();
    if (!client) return;

    // Read the request
    String req = client.readStringUntil('\r');
    client.flush();

    // --- Scalable Command Processing ---
    // Process irrigation zone commands using a loop
    for (int i = 0; i < NUM_IRRIGATION_ZONES; i++) {
        if (req.indexOf("/zone/" + String(i + 1) + "/on") > 0) _irrigationControl.manualControl(i, true);
        else if (req.indexOf("/zone/" + String(i + 1) + "/off") > 0) _irrigationControl.manualControl(i, false);
    }

    // Process fan commands using a loop
    for (int i = 0; i < NUM_FANS; i++) {
        if (req.indexOf("/fan/" + String(i + 1) + "/on") > 0) _climateControl.controlFan(i, true);
        else if (req.indexOf("/fan/" + String(i + 1) + "/off") > 0) _climateControl.controlFan(i, false);
    }

    // Trap controls
    if      (req.indexOf("/trap/open") > 0) _trapControl.open();
    else if (req.indexOf("/trap/close")> 0) _trapControl.close();
    else if (req.indexOf("/trap/up")   > 0) _trapControl.up();
    else if (req.indexOf("/trap/down") > 0) _trapControl.down();
    else if (req.indexOf("/trap/stop") > 0) _trapControl.stop();
    else if (req.indexOf("/trap/reset") > 0) _trapControl.resetErrors();

    // Scalable door controls
    for (int i = 0; i < NUM_OF_DOORS; i++) {
        String p = "/door/" + String(i + 1);
        if      (req.indexOf(p + "/open") > 0) _doorControl.open(i);
        else if (req.indexOf(p + "/close") > 0) _doorControl.close(i);
        else if (req.indexOf(p + "/up") > 0) _doorControl.up(i);
        else if (req.indexOf(p + "/down") > 0) _doorControl.down(i);
        else if (req.indexOf(p + "/stop") > 0) _doorControl.stop(i);
        else if (req.indexOf(p + "/reset") > 0) _doorControl.resetErrors(i);
    }

    // Send HTML response
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html; charset=UTF-8");
    client.println();
    client.println(R"rawliteral(
        <!DOCTYPE html>
        <html lang="en">
        <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
        <title>Solar Control Panel</title>
        <link rel="stylesheet" href="https://fonts.googleapis.com/css?family=Roboto:400,700&display=swap">
        <style>
            body {
            font-family: 'Roboto', Arial, sans-serif;
            margin: 0;
            padding: 0;
            background: #e9f5ff;
            color: #222;
            padding-top: 60px; /* Space for the fixed top bar */
            }
            .container {
            max-width: 900px;
            margin: 0 auto;
            padding: 16px;
            }
            h1 {
            text-align: center;
            color: #007BFF;
            margin-top: 0; /* Remove default top margin */
            margin-bottom: 24px;
            }
            .grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(320px, 1fr));
            gap: 20px;
            }
            .card {
            background: #fff;
            border-radius: 12px;
            box-shadow: 0 2px 8px rgba(0,0,0,0.08);
            padding: 20px;
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
            .modal {
                display: none; /* Hidden by default */
                position: fixed; /* Stay in place */
                z-index: 1000;
                left: 0; top: 0;
                width: 100%; height: 100%;
                overflow: auto; /* Enable scroll if needed */
                background-color: rgba(0,0,0,0.5); /* Black w/ opacity */
                padding-top: 60px;
            }
            .modal-content {
                background-color: #fefefe;
                margin: 5% auto;
                padding: 20px;
                border: 1px solid #888;
                border-radius: 12px;
                width: 80%;
                max-width: 500px;
                box-shadow: 0 4px 15px rgba(0,0,0,0.2);
            }
            .close-button {
                color: #aaa;
                float: right;
                font-size: 28px;
                font-weight: bold;
            }
            .close-button:hover,
            .close-button:focus {
                color: black;
                text-decoration: none;
                cursor: pointer;
            }
            .top-bar {
                position: fixed;
                top: 0;
                left: 0;
                width: 100%;
                background: #007BFF;
                color: white;
                padding: 10px 20px;
                display: flex;
                justify-content: space-between;
                align-items: center;
                box-shadow: 0 2px 5px rgba(0,0,0,0.2);
                z-index: 1001;
                box-sizing: border-box;
            }
            .top-bar a.btn {
                margin: 0;
                background: #fff;
                color: #007BFF;
                font-weight: bold;
            }
            .top-bar-time {
                font-size: 0.9em;
            }
            .top-bar-left {
                display: flex;
                align-items: center;
                gap: 24px;
            }
            .top-bar-status {
                font-size: 0.9em;
                font-weight: bold;
                padding: 4px 10px;
                border-radius: 12px;
                background-color: rgba(255, 255, 255, 0.2);
            }
            .top-bar-status.rain {
                background-color: #ffc107;
                color: #333;
            }
            .zone-grid { display: flex; flex-direction: column; gap: 16px; }
            .zone-control { border: 1px solid #eee; padding: 12px; border-radius: 8px; }
            .zone-control h4 { margin-top: 0; margin-bottom: 8px; color: #333; }
            .zone-control .status { font-size: 1em; font-weight: normal; color: #555; }
            .zone-control .status b { font-weight: bold; color: #333;
            }
        </style>
        <script>
            document.addEventListener('DOMContentLoaded', function() {
                const modal = document.getElementById('infoModal');
                const infoBtn = document.getElementById('infoBtn');
                const closeBtn = document.querySelector('.close-button');

                // Show the modal when the info button is clicked
                infoBtn.onclick = function(e) {
                    e.preventDefault(); // Prevent page jump from '#' href
                    modal.style.display = 'block';
                }

                // Hide the modal when the close button is clicked
                closeBtn.onclick = function() {
                    modal.style.display = 'none';
                }

                // Hide modal if user clicks on the background overlay
                window.onclick = function(event) {
                    if (event.target == modal) { modal.style.display = 'none'; }
                }
            });
            // Auto-refresh the page every 10 seconds to keep data fresh
            setTimeout(() => { window.location.reload(); }, 10000);
        </script>
        </head>
        <body>
        <div class="top-bar">
            <div class="top-bar-left">
                <div class='top-bar-time'><b>Date & Time:</b> )rawliteral");
    client.print(rtcTime);
    client.println(R"rawliteral(</div>)rawliteral");
    String rainStatusClass = (rainStatus == "raining") ? " rain" : "";
    client.print("<div class='top-bar-status" + rainStatusClass + "'><b>Rain:</b> ");
    client.print(rainStatus);
    client.println(R"rawliteral(</div>
            </div>
    )rawliteral");
    client.println(R"rawliteral(
            <a href="#" id="infoBtn" class="btn">System Info</a>
        </div>
        <!-- The Modal -->
        <div id="infoModal" class="modal">
          <div class="modal-content">
            <span class="close-button">&times;</span>
            <h2>System Initialization</h2>)rawliteral");
    client.println("<p><strong>Firmware Version:</strong> " + String(FIRMWARE_VERSION) + "</p>");
    client.println("<p><strong>Build Environment:</strong> " + String(BUILD_ENV_NAME) + "</p>");
    client.println(R"rawliteral(<hr style='border:none;border-top:1px solid #eee;margin:16px 0;'><h3>WiFi Connection</h3>)rawliteral");
    if (WiFi.status() == WL_CONNECTED) {
        client.println("<p><strong>Status:</strong> <span class='status ok'>Connected</span></p>");
        client.println(String("<p><strong>SSID:</strong> ") + WiFi.SSID() + "</p>");
        client.println("<p><strong>IP Address:</strong> " + WiFi.localIP().toString() + "</p>");
        client.println("<p><strong>Signal Strength:</strong> " + String(WiFi.RSSI()) + " dBm</p>");
    } else {
        client.println("<p><strong>Status:</strong> <span class='status err'>Offline Mode</span></p>");
        client.println("<p><em>WiFi details not available.</em></p>");
    }
    client.println(R"rawliteral(<hr style='border:none;border-top:1px solid #eee;margin:16px 0;'><h3>I2C Devices</h3>)rawliteral");
    client.println("<p>" + i2cScanResults + "</p>");
    client.println(R"rawliteral(<hr style='border:none;border-top:1px solid #eee;margin:16px 0;'><h3>Hardware Status</h3>)rawliteral");
    client.print("<p><strong>RTC (Clock):</strong> ");
    client.println(rtcStatus ? "<span class='status ok'>OK</span></p>" : "<span class='status err'>Not Found</span></p>");
    client.print("<p><strong>SD Card:</strong> ");
    client.println(sdStatus ? "<span class='status ok'>OK</span></p>" : "<span class='status err'>Not Found</span></p>");
    client.print("<p><strong>PCF8574_1 (0x20):</strong> ");
    client.println(pcf1Status ? "<span class='status ok'>OK</span></p>" : "<span class='status err'>Not Found</span></p>");
    client.print("<p><strong>PCF8574_2 (0x21):</strong> ");
    client.println(pcf2Status ? "<span class='status ok'>OK</span></p>" : "<span class='status err'>Not Found</span></p>");
    client.print("<p><strong>Multiplexer (MUX):</strong> ");
    client.println(muxStatus ? "<span class='status ok'>OK</span></p>" : "<span class='status err'>Failed/Check Wiring</span></p>");
    client.println(R"rawliteral(
          </div>
        </div>
        <div class="container">
            <h1>☀️ Solar Control Panel</h1>)rawliteral");
    client.println(R"rawliteral(
            <div class="grid">

            <!-- Climate and Humidity Card -->
            <div class="card">
                <h2><span class="icon">🌡️</span>Climate &amp; Humidity</h2>)rawliteral");
    client.print("<div class='status ok'><b>Temp Interior:</b> "); client.print(sensors.tempInt, 1); client.println(" &deg;C</div>");
    client.print("<div class='status ok'><b>Temp Exterior:</b> "); client.print(sensors.tempExt, 1); client.println(" &deg;C</div>");
    client.print("<div class='status ok'><b>Humid Interior:</b> "); client.print(sensors.humInt, 1); client.println(" %</div>");
    client.print("<div class='status ok'><b>Humid Exterior:</b> "); client.print(sensors.humExt, 1); client.println(" %</div>");
    client.println(R"rawliteral(
            </div>

            <!-- Soil Moisture Card -->
            <div class="card">
                <h2><span class="icon">🌱</span>Soil Moisture</h2>)rawliteral");
    for (int i = 0; i < NUM_SOIL_SENSORS; i++) {
        client.print("<div class='status'><b>Soil " + String(i + 1) + ":</b> " + soilStatus[i] + " (" + String(sensors.soilMoisture[i]) + ")</div>");
    }
    client.println(R"rawliteral(
            </div>

            <!-- Irrigation Control Card -->
            <div class="card">
                <h2><span class="icon">💧</span>Irrigation Control</h2>
                <div class="zone-grid">)rawliteral");
    for (int i = 0; i < NUM_IRRIGATION_ZONES; i++) {
        const IrrigationZone& zone = IRRIGATION_ZONES[i];
        String zoneNum = String(i + 1);
        client.println("<div class='zone-control'>");
        client.println("<h4>Zone " + zoneNum + "</h4>");
        client.println("<div class='status'><b>Soil:</b> " + soilStatus[zone.soilSensorIndex] + " (" + String(sensors.soilMoisture[zone.soilSensorIndex]) + ")</div>");
        client.println("<div class='status'><b>Pump " + String(zone.pumpIndex + 1) + ":</b> " + pumpStatus[zone.pumpIndex] + "</div>");
        client.println("<div class='status'><b>Valve " + String(zone.valveIndex + 1) + ":</b> " + valveStatus[zone.valveIndex] + "</div>");
        client.print("<div><a href='/zone/" + zoneNum + "/on' class='btn'>ON</a>");
        client.print("<a href='/zone/" + zoneNum + "/off' class='btn off'>OFF</a></div>");
        client.println("</div>");
    }
    client.println(R"rawliteral(
                </div>
            </div>

            <!-- Fans Card -->
            <div class="card">
                <h2><span class="icon">🌀</span>Fans</h2>)rawliteral");
    for (int i = 0; i < NUM_FANS; i++) {
        String fanNum = String(i + 1);
        String p = "/fan/" + fanNum;
        if (i > 0) client.println("<hr style='border:none;border-top:1px solid #eee;margin:16px 0;'>");
        client.println("<h4>Fan " + fanNum + "</h4>");
        client.println("<div class='status'><b>Status:</b> " + fanStatus[i] + "</div>");
        client.println("<div><a href='" + p + "/on' class='btn'>ON</a><a href='" + p + "/off' class='btn off'>OFF</a></div>");
    }
    client.println(R"rawliteral(
            </div>

            <!-- Ventilation Trap Card -->
            <div class="card">
                <h2><span class="icon">🪟</span>Ventilation Trap</h2>)rawliteral");
    client.println("<div class='status'><b>Status:</b> " + trapStatus + "</div>");
    if (trapStatus == "error" || trapStatus == "disabled") {
        client.println("<div><a href='/trap/reset' class='btn stop'>Reset Errors</a></div>");
    }
    client.println(R"rawliteral(
                <div><a href='/trap/open' class='btn'>Auto Open</a><a href='/trap/close' class='btn off'>Auto Close</a></div>
                <div><a href='/trap/up' class='btn up'>Manual Up</a><a href='/trap/stop' class='btn stop'>Manual Stop</a><a href='/trap/down' class='btn down'>Manual Down</a></div>
    )rawliteral");
    client.println(R"rawliteral(
            </div>

            <!-- Doors Card -->
            <div class="card">
                <h2><span class="icon">🚪</span>Doors</h2>)rawliteral");
    for (int i = 0; i < NUM_OF_DOORS; i++) {
        String doorNum = String(i + 1);
        String p = "/door/" + doorNum;
        if (i > 0) client.println("<hr style='border:none;border-top:1px solid #eee;margin:16px 0;'>");
        client.println("<h4>Door " + doorNum + "</h4>");

        String statusClass = "";
        if (doorStatus[i] == "error" || doorStatus[i] == "disabled") {
            statusClass = " err";
        } else if (doorStatus[i] == "moving" || doorStatus[i] == "stopped") {
            statusClass = " warn";
        }
        client.println("<div class='status" + statusClass + "'><b>Status:</b> " + doorStatus[i] + "</div>");

        if (doorStatus[i] == "error" || doorStatus[i] == "disabled") {
            client.println("<div><a href='" + p + "/reset' class='btn stop'>Reset Errors</a></div>");
        }
        client.println("<div><a href='" + p + "/open' class='btn'>Auto Open</a><a href='" + p + "/close' class='btn off'>Auto Close</a></div>");
        client.println("<div><a href='" + p + "/up' class='btn up'>Manual Up</a><a href='" + p + "/stop' class='btn stop'>Manual Stop</a><a href='" + p + "/down' class='btn down'>Manual Down</a></div>");
    }
    client.println(R"rawliteral(
            </div>

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