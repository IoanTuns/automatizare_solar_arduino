#include <Arduino.h>
#include <WiFiS3.h>
#include "SolarWebServer.h"
#include "config.h" // Include the header with function prototypes
#include "WebAuthentication.h"  // Add this line
#include "SecureCredentials.h"  // Add this line
#include "WebPages.h" // Include the new header for web content

/**
 * @file SolarWebServer.cpp
 * @brief Implementation of the SolarWebServer class, which manages a web server for the garden automation system.
 * 
 * This file contains the implementation of the SolarWebServer class methods, which handle the initialization
 * and operation of a web server that provides real-time sensor data and system status to connected clients.
 */

/**
 * @brief Constructor for SolarWebServer with security features
 */
SolarWebServer::SolarWebServer(uint16_t port, PumpControl& pumpControl, DoorControl& doorControl, 
                               ClimateControl& climateControl, ValveControl& valveControl, 
                               TrapControl& trapControl, IrrigationControl& irrigationControl)
    : _server(port),
      _pumpControl(pumpControl),
      _doorControl(doorControl),
      _valveControl(valveControl),
      _climateControl(climateControl),
      _trapControl(trapControl),
      _irrigationControl(irrigationControl) {}

/**
 * @brief Initialize authentication system
 * @param username Admin username
 * @param password Admin password
 */
void SolarWebServer::initializeAuthentication(const char* username, const char* password) {
    _adminUsername = username;
    _adminPassword = password;
    WebAuthentication::init(username, password);
}

/**
 * @brief Start server with WiFi credentials
 * @param ssid WiFi SSID
 * @param pass WiFi password
 */
void SolarWebServer::begin(const char* ssid, const char* pass) {
    // Check for WiFi module
    if (WiFi.status() == WL_NO_MODULE) {
        Serial.println("ERROR: Communication with WiFi module failed!");
        return;
    }

    // Store credentials
    if (!CredentialsStorage::storeCredentials(ssid, pass)) {
        Serial.println("Warning: Failed to store WiFi credentials");
    }

    // Connect to WiFi
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
        Serial.println("\n[Web] Secure server started, IP: " + WiFi.localIP().toString());
    } else {
        Serial.println("\n[Web] Failed to connect to WiFi. Continuing in offline mode.");
    }
}

/**
 * @brief Start server with stored credentials
 */
void SolarWebServer::begin() {
    char ssid[33];
    char pass[65];
    if (CredentialsStorage::loadCredentials(ssid, pass)) {
        begin(ssid, pass);
        // Clear sensitive data from stack
        memset(ssid, 0, sizeof(ssid));
        memset(pass, 0, sizeof(pass));
    } else {
        Serial.println("[Web] No valid stored credentials found. Server not started.");
    }
}

/**
 * @brief Print current WiFi status information
 */
void SolarWebServer::printWifiStatus() {
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
    
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);
    
    long rssi = WiFi.RSSI();
    Serial.print("Signal strength (RSSI): ");
    Serial.print(rssi);
    Serial.println(" dBm");
}

/**
 * @brief Clean up expired sessions (call periodically)
 */
void SolarWebServer::cleanupSessions() {
    WebAuthentication::cleanupExpiredSessions();
}

/**
 * @brief Send security headers with HTTP response
 * @param client WiFi client connection
 * @param sessionId Optional session ID to set as cookie
 */
void SolarWebServer::sendSecurityHeaders(WiFiClient& client, const String& sessionId) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html; charset=UTF-8");
    if (!sessionId.isEmpty()) {
        client.println("Set-Cookie: sessionId=" + sessionId + "; HttpOnly; SameSite=Strict");
    }
    client.println("X-Content-Type-Options: nosniff");
    client.println("X-Frame-Options: DENY");
    client.println("X-XSS-Protection: 1; mode=block");
    client.println("Connection: close");
    client.println();
}

/**
 * @brief Send unauthorized response with login prompt
 * @param client WiFi client connection
 */
void SolarWebServer::sendUnauthorizedResponse(WiFiClient& client) {
    client.println("HTTP/1.1 401 Unauthorized");
    client.println("Content-Type: text/html; charset=UTF-8");
    client.println("Connection: close");
    client.println();
    client.println("<!DOCTYPE html>");
    client.println("<html>");
    client.println("<head>");
    client.println("    <title>Authentication Required</title>");
    client.println("    <style>");
    client.println("        body { font-family: Arial, sans-serif; text-align: center; margin-top: 50px; }");
    client.println("        .error { color: #dc3545; }");
    client.println("    </style>");
    client.println("</head>");
    client.println("<body>");
    client.println("    <h1>Authentication Required</h1>");
    client.println("    <p class=\"error\">Please provide valid credentials to access the Greenhouse System.</p>");
    client.println("</body>");
    client.println("</html>");
}

/**
 * @brief Send login page
 * @param client WiFi client connection
 * @param showError Show error message for failed login
 */
void SolarWebServer::sendLoginPage(WiFiClient& client, bool showError) {
    client.println("<html><body>");
    client.println("<h2>[SECURE] Solar Control Panel</h2>");
    if (showError) {
        client.println("<div style='color:red;'>Invalid username or password. Please try again.</div>");
    }
    client.print(FPSTR(LOGIN_PAGE_FORM));
    client.println("</body></html>");
}

/**
 * @brief Send error page
 * @param client WiFi client connection
 * @param errorCode HTTP error code
 * @param message Error message
 */
void SolarWebServer::sendErrorPage(WiFiClient& client, int errorCode, const String& message) {
    client.println("HTTP/1.1 " + String(errorCode) + " " + message);
    client.println("Content-Type: text/html; charset=UTF-8");
    client.println("Connection: close");
    client.println();
    client.println("<!DOCTYPE html><html><head><title>Error</title></head>");
    client.println("<body><h1>Error " + String(errorCode) + "</h1>");
    client.println("<p>" + message + "</p></body></html>");
}

/**
 * @brief Parse authorization header from request
 * @param request HTTP request string
 * @return Authorization header value
 */
String SolarWebServer::parseAuthHeader(const String& request) {
    int authIndex = request.indexOf("Authorization: ");
    if (authIndex == -1) return "";
    
    int lineEnd = request.indexOf('\n', authIndex);
    if (lineEnd == -1) lineEnd = request.length();
    
    return request.substring(authIndex + 15, lineEnd);
}

/**
 * @brief Parse session cookie from request
 * @param request HTTP request string
 * @return Session ID from cookie
 */
String SolarWebServer::parseSessionCookie(const String& request) {
    int cookieIndex = request.indexOf("Cookie: ");
    if (cookieIndex == -1) return "";
    
    int sessionIndex = request.indexOf("sessionId=", cookieIndex);
    if (sessionIndex == -1) return "";
    
    int sessionStart = sessionIndex + 10;
    int sessionEnd = request.indexOf(';', sessionStart);
    if (sessionEnd == -1) {
        // A cookie header line ends with \r\n. We need to find the \r to correctly terminate the string.
        sessionEnd = request.indexOf('\r', sessionStart);
    }
    if (sessionEnd == -1) {
        sessionEnd = request.length(); // Fallback if it's the very end of the request string
    }
    
    return request.substring(sessionStart, sessionEnd);
}

/**
 * @brief Parse request path from HTTP request
 * @param request HTTP request string
 * @return Request path
 */
String SolarWebServer::parseRequestPath(const String& request) {
    int pathStart = request.indexOf(' ') + 1;
    int pathEnd = request.indexOf(' ', pathStart);
    if (pathEnd == -1) pathEnd = request.indexOf('\n', pathStart);
    
    return request.substring(pathStart, pathEnd);
}

/**
 * @brief Parse form data from POST request body
 * @param request HTTP request string
 * @return Form data string
 */
String SolarWebServer::parseFormData(const String& request) {
    int bodyStart = request.indexOf("\r\n\r\n");
    if (bodyStart == -1) return "";
    
    return request.substring(bodyStart + 4);
}

String urlDecode(const String& text) {
  String decoded = "";
  char temp[] = "0x00";
  for (unsigned int i = 0; i < text.length(); i++) {
    if (text[i] == '%') {
      if (i + 2 < text.length()) {
        temp[2] = text[i+1];
        temp[3] = text[i+2];
        decoded += (char) strtol(temp, NULL, 16);
        i += 2;
      }
    } else if (text[i] == '+') {
      decoded += ' ';
    } else {
      decoded += text[i];
    }
  }
  return decoded;
}

/**
 * @brief Parse a specific field from form data
 * @param formData URL-encoded form data
 * @param fieldName Name of the field to extract
 * @return Field value (URL-decoded)
 */
String SolarWebServer::parseFormField(const String& formData, const String& fieldName) {
    String searchPattern = fieldName + "=";
    int fieldStart = formData.indexOf(searchPattern);
    if (fieldStart == -1) return "";
    
    fieldStart += searchPattern.length();
    int fieldEnd = formData.indexOf('&', fieldStart);
    if (fieldEnd == -1) fieldEnd = formData.length();
    
    String value = formData.substring(fieldStart, fieldEnd);
    
    value = urlDecode(value);

    // Reject suspicious input
    if (value.indexOf("<") != -1 || value.indexOf(">") != -1) return "";
    
    return value;
}

/**
 * @brief Validate incoming request for security
 * @param request HTTP request string
 * @return true if request is valid, false otherwise
 */
bool SolarWebServer::isValidRequest(const String& request) {
    // Check request size
    if (request.length() > MAX_REQUEST_SIZE) {
        return false;
    }
    
    // Check for basic HTTP structure
    if (!request.startsWith("GET ") && !request.startsWith("POST ")) {
        return false;
    }
    
    // Check for suspicious patterns
    if (request.indexOf("../") != -1 || 
        request.indexOf("<script") != -1 || 
        request.indexOf("javascript:") != -1) {
        return false;
    }
    
    return true;
}

/**
 * @brief Process control commands from authenticated requests
 * @param path The request path from the client
 */
void SolarWebServer::processControlCommands(const String& path) {
    // Process irrigation zone commands
    for (int i = 0; i < NUM_IRRIGATION_ZONES; i++) {
        String zonePath = "/zone/" + String(i + 1);
        if (path == zonePath + "/on") {
            _irrigationControl.manualControl(i, true);
        } else if (path == zonePath + "/off") {
            _irrigationControl.manualControl(i, false);
        }
    }

    // Process fan commands
    for (int i = 0; i < NUM_FANS; i++) {
        String fanPath = "/fan/" + String(i + 1);
        if (path == fanPath + "/on") {
            _climateControl.controlFan(i, true);
        } else if (path == fanPath + "/off") {
            _climateControl.controlFan(i, false);
        }
    }

    // Trap controls
    if (path == "/trap/open") _trapControl.open();
    else if (path == "/trap/close") _trapControl.close();
    else if (path == "/trap/up") _trapControl.up();
    else if (path == "/trap/down") _trapControl.down();
    else if (path == "/trap/stop") _trapControl.stop();
    else if (path == "/trap/reset") _trapControl.resetErrors();

    // Door controls
    for (int i = 0; i < NUM_OF_DOORS; i++) {
        String doorPath = "/door/" + String(i + 1);
        if (path == doorPath + "/open") _doorControl.open(i);
        else if (path == doorPath + "/close") _doorControl.close(i);
        else if (path == doorPath + "/up") _doorControl.up(i);
        else if (path == doorPath + "/down") _doorControl.down(i);
        else if (path == doorPath + "/stop") _doorControl.stop(i);
        else if (path == doorPath + "/reset") _doorControl.resetErrors(i);
    }
}

/**
 * @brief Handle incoming client requests with security
 * @param sensors Current sensor data
 * @param rtcTime Current time string
 */
void SolarWebServer::handleClient(const SensorData& sensors, const String& rtcTime) {
    WiFiClient client = _server.available();
    if (!client) return;

    String request = "";
    unsigned long clientStart = millis();

    // Read the HTTP request (headers only, minimal parsing)
    while (client.connected() && (millis() - clientStart < 2000)) {
        if (client.available()) {
            char c = client.read();
            request += c;
            // Stop reading after headers (double newline)
            if (request.endsWith("\r\n\r\n") || request.endsWith("\n\n")) break;
        }
    }

    // --- EARLY SESSION VALIDATION ---
    String sessionId = parseSessionCookie(request);
    if (!WebAuthentication::validateSession(sessionId)) {
        sendLoginPage(client, false); // <-- Change 'true' to 'false'
        client.stop();
        return;
    }

    // Proceed with request parsing and control logic only if session is valid
    String path = parseRequestPath(request);

    bool isApiCall = path.startsWith("/zone/") || path.startsWith("/fan/") || path.startsWith("/trap/") || path.startsWith("/door/");

    // Check for a valid session for all other pages
    if (WebAuthentication::validateSession(sessionId)) {
        // If it's an API call, process the command first.
        if (isApiCall) {
            processControlCommands(path);
        }
        // For any valid, authenticated request, render the main page.
        // This provides immediate feedback for API calls by sending the updated page.
        sendMainPage(client, sensors, rtcTime, ""); // Pass empty string for session ID, as it's already set
    } else {
        if (isApiCall) {
            sendUnauthorizedResponse(client);
        } else {
            // If no valid session, and not a login attempt, show the login page.
            sendLoginPage(client, false);
        }
    }

    client.stop();
}


/**
 * @brief Send main control page with sensor data
 * @param client WiFi client connection
 * @param sensors Current sensor data
 * @param rtcTime Current time string
 * @param newSessionId New-session ID to set as cookie (optional)
 */
void SolarWebServer::sendMainPage(WiFiClient& client, const SensorData& sensors, const String& rtcTime, const String& newSessionId)
{
    sendMainPageHeader(client, newSessionId, rtcTime);

    client.print(FPSTR(GRID_START));
    sendClimateCard(client, sensors);
    sendSoilMoistureCard(client, sensors);
    sendIrrigationCard(client);
    sendFansCard(client);
    sendVentilationTrapCard(client);
    sendDoorsCard(client);
    client.print(FPSTR(GRID_END));

    sendMainPageFooter(client);
}

void SolarWebServer::sendMainPageHeader(WiFiClient &client, const String &newSessionId, const String& rtcTime)
{
    sendSecurityHeaders(client, newSessionId);
    client.print(FPSTR(MAIN_PAGE_HEADER_START));
    client.print(FPSTR(MAIN_PAGE_SCRIPT));
    client.print(FPSTR(MAIN_PAGE_HEADER_END));

    // --- Top Bar ---
    client.println(F("<div class='top-bar'>")); // This is now outside the main container
    client.println(F("  <div class='top-bar-left'>"));
    client.print(F("    <div class='top-bar-time'><b>Date & Time:</b> "));
    client.print(rtcTime);
    client.println(F("</div>"));
    String rainStatusClass = (rainStatus == "raining") ? " rain" : "";
    client.print(F("    <div class='top-bar-status"));
    client.print(rainStatusClass);
    client.print(F("'><b>Rain:</b> "));
    client.print(rainStatus);
    client.println(F("</div>"));
    client.println(F("  </div>"));
    client.println(F("  <a href='#' id='infoBtn' class='btn'>System Info</a>"));
    client.println(F("</div>"));

    // --- The System Info Modal ---
    client.println(F("<div id='infoModal' class='modal'>"));
    client.println(F("  <div class='modal-content'>"));
    client.println(F("    <span class='close-button'>&times;</span>"));
    client.println(F("    <h2>System Initialization</h2>"));
    client.println("<p><strong>Firmware Version:</strong> " + String(FIRMWARE_VERSION) + "</p>");
    client.println("<p><strong>Build Environment:</strong> " + String(BUILD_ENV_NAME) + "</p>");
    client.println(F("<hr style='border:none;border-top:1px solid #eee;margin:16px 0;'><h3>WiFi Connection</h3>"));
    if (WiFi.status() == WL_CONNECTED) {
        client.println(F("<p><strong>Status:</strong> <span class='status ok'>Connected</span></p>"));
        client.println(String("<p><strong>SSID:</strong> ") + WiFi.SSID() + "</p>");
        client.println("<p><strong>IP Address:</strong> " + WiFi.localIP().toString() + "</p>");
        client.println("<p><strong>Signal Strength:</strong> " + String(WiFi.RSSI()) + " dBm</p>");
    } else {
        client.println(F("<p><strong>Status:</strong> <span class='status err'>Offline Mode</span></p>"));
        client.println(F("<p><em>WiFi details not available.</em></p>"));
    }
    client.println(F("<hr style='border:none;border-top:1px solid #eee;margin:16px 0;'><h3>I2C Devices</h3>"));
    client.println("<p>" + i2cScanResults + "</p>");
    client.println(F("<hr style='border:none;border-top:1px solid #eee;margin:16px 0;'><h3>Hardware Status</h3>"));

    auto printStatus = [&](const String& name, bool status, const char* failMsg = "Not Found") {
        client.print("<p><strong>" + name + ":</strong> ");
        client.println(status ? "<span class='status ok'>OK</span></p>" : "<span class='status err'>" + String(failMsg) + "</span></p>");
    };

    printStatus("RTC (Clock)", rtcStatus);
    printStatus("SD Card", sdStatus);
    printStatus("PCF8574_1 (0x20)", pcf1Status);
    printStatus("PCF8574_2 (0x21)", pcf2Status);
    printStatus("Multiplexer (MUX)", muxStatus, "Failed/Check Wiring");

    client.println(F("  </div>")); // close modal-content
    client.println(F("</div>"));   // close modal

    // Open the main container and print the title
    client.println(F("<div class='container'>"));
    client.println(F("<h1>[SECURE] Solar Control Panel</h1>"));
}

void SolarWebServer::sendClimateCard(WiFiClient &client, const SensorData &sensors)
{
    client.print(FPSTR(CARD_START));
    client.println(F("<h2>[TEMP] Climate</h2>"));
    client.print("<div class='status'><strong>Interior Temp:</strong> ");
    client.print(sensors.tempInt, 1);
    client.println(" &deg;C</div>");
    client.print("<div class='status'><strong>Exterior Temp:</strong> ");
    client.print(sensors.tempExt, 1);
    client.println(" &deg;C</div>");
    client.print("<div class='status'><strong>Interior Humidity:</strong> ");
    client.print(sensors.humInt, 1);
    client.println(" %</div>");
    client.print("<div class='status'><strong>Exterior Humidity:</strong> ");
    client.print(sensors.humExt, 1);
    client.println(" %</div>");
    client.print(FPSTR(CARD_END));
}

void SolarWebServer::sendSoilMoistureCard(WiFiClient &client, const SensorData &sensors)
{
    client.print(FPSTR(CARD_START));
    client.println(F("<h2>[SOIL] Soil Moisture</h2>"));
    for (int i = 0; i < NUM_SOIL_SENSORS; i++)
    {
        client.print("<div class='status'><strong>Soil " + String(i + 1) + ":</strong> ");
        client.print(String(sensors.soilMoisture[i]));
        client.println("</div>");
    }
    client.print(FPSTR(CARD_END));
}

void SolarWebServer::sendIrrigationCard(WiFiClient &client)
{
    client.print(FPSTR(CARD_START));
    client.println(F("<h2>[WATER] Irrigation Control</h2>"));
    for (int i = 0; i < NUM_IRRIGATION_ZONES; i++)
    {
        String zoneNum = String(i + 1);
        client.println("<div style='border: 1px solid #eee; padding: 10px; margin: 10px 0; border-radius: 5px;'>");
        client.println("<h4>Zone " + zoneNum + "</h4>");
        client.print(F("<div class='status'><strong>Status:</strong> "));
        client.print(soilStatus[IRRIGATION_ZONES[i].soilSensorIndex]);
        client.print(F(" (Pump: ")); client.print(pumpStatus[IRRIGATION_ZONES[i].pumpIndex]);
        client.print(F(", Valve: ")); client.print(valveStatus[IRRIGATION_ZONES[i].valveIndex]);
        client.println(F(")</div>"));

        renderButton(client, "/zone/" + zoneNum + "/on", "ON", "");
        renderButton(client, "/zone/" + zoneNum + "/off", "OFF", "off");

        client.println("</div>");
    }
    client.print(FPSTR(CARD_END));
}

void SolarWebServer::sendFansCard(WiFiClient &client)
{
    client.print(FPSTR(CARD_START));
    client.println(F("<h2>[FAN] Fans</h2>"));
    for (int i = 0; i < NUM_FANS; i++)
    {
        String fanNum = String(i + 1);
        client.println("<div>");
        client.print(F("<h4>Fan ")); client.print(fanNum);
        client.print(F(" - Status: <span class='status "));
        client.print(String(fanStatus[i]) == "on" ? "ok" : "err");
        client.print(F("'>")); client.print(fanStatus[i]); client.println(F("</span></h4>"));

        renderButton(client, "/fan/" + fanNum + "/on", "Turn ON", "on");
        renderButton(client, "/fan/" + fanNum + "/off", "Turn OFF", "off");
        client.println("</div>");
    }
    client.print(FPSTR(CARD_END));
}

void SolarWebServer::sendVentilationTrapCard(WiFiClient &client)
{
    client.print(FPSTR(CARD_START));
    client.println(F("<h2>[VENT] Ventilation Trap</h2>"));
    client.print(F("<div class='status'><strong>Status:</strong> <span class='status "));
    if (trapStatus == "open" || trapStatus == "closed") client.print("ok");
    else if (trapStatus == "error" || trapStatus == "disabled") client.print("err");
    client.print(F("'>")); client.print(trapStatus); client.println(F("</span></div>"));

    client.println(F("<div><button onclick=\"sendCommand('/trap/open')\" class='btn'>Auto Open</button><button onclick=\"sendCommand('/trap/close')\" class='btn off'>Auto Close</button></div>"));
    client.println(F("<div style='margin-top: 10px;'><button onclick=\"sendCommand('/trap/up')\" class='btn up'>Manual Up</button><button onclick=\"sendCommand('/trap/stop')\" class='btn stop'>Stop</button><button onclick=\"sendCommand('/trap/down')\" class='btn down'>Manual Down</button></div>"));
    client.println(F("<div style='margin-top: 10px;'><button onclick=\"sendCommand('/trap/reset')\" class='btn stop'>Reset Errors</button></div>"));
    client.print(FPSTR(CARD_END));
}

void SolarWebServer::sendDoorsCard(WiFiClient &client)
{
    client.print(FPSTR(CARD_START));
    client.println(F("<h2>[DOOR] Doors</h2>"));
    for (int i = 0; i < NUM_OF_DOORS; i++)
    {
        String doorNum = String(i + 1);
        client.println("<div style='border: 1px solid #eee; padding: 10px; margin: 10px 0; border-radius: 5px;'>");
        client.println("<h4>Door " + doorNum + "</h4>");
        client.println("<div><button onclick=\"sendCommand('/door/" + doorNum + "/open')\" class='btn'>Auto Open</button><button onclick=\"sendCommand('/door/" + doorNum + "/close')\" class='btn off'>Auto Close</button></div>");
        client.println("<div style='margin-top: 5px;'><button onclick=\"sendCommand('/door/" + doorNum + "/up')\" class='btn up'>Manual Up</button><button onclick=\"sendCommand('/door/" + doorNum + "/stop')\" class='btn stop'>Stop</button><button onclick=\"sendCommand('/door/" + doorNum + "/down')\" class='btn down'>Manual Down</button></div>");
        client.println("<div style='margin-top: 5px;'><button onclick=\"sendCommand('/door/" + doorNum + "/reset')\" class='btn stop'>Reset Errors</button></div>");
        client.println("</div>");
    }
    client.print(FPSTR(CARD_END));
}

void SolarWebServer::sendMainPageFooter(WiFiClient &client)
{
    client.print(FPSTR(MAIN_PAGE_FOOTER));
}

void SolarWebServer::renderButton(WiFiClient& client, const String& path, const String& label, const String& cssClass) {
    client.print("<button onclick=\"sendCommand('");
    client.print(path);
    client.print("')\" class='btn ");
    client.print(cssClass);
    client.print("\">");
    client.print(label);
    client.println("</button>");
}

void SolarWebServer::handleLoginRequest(WiFiClient& client, const String& request, const SensorData& sensors, const String& rtcTime) {
    String formData = parseFormData(request);
    String username = parseFormField(formData, "username");
    String password = parseFormField(formData, "password");

    // WARNING: Plaintext password comparison. This is not secure.
    if (username == _adminUsername && password == _adminPassword) {
        String sessionId = WebAuthentication::createSession();
        sendSecurityHeaders(client, sessionId);
        sendMainPage(client, sensors, rtcTime, sessionId);
    } else {
        sendLoginPage(client, true); // Only show error after failed login
    }
}