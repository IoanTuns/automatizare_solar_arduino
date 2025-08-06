#ifndef SOLARWEBSERVER_H
#define SOLARWEBSERVER_H

#include <WiFiS3.h>
#include "SensorData.h"
#include "PumpControl.h"
#include "DoorControl.h"
#include "ClimateControl.h"
#include "IrrigationControl.h"
#include "TrapControl.h"

#define MAX_REQUEST_SIZE 2048

class SolarWebServer {
public:
    SolarWebServer(uint16_t port, PumpControl& pumpControl, DoorControl& doorControl, 
                   ClimateControl& climateControl, ValveControl& valveControl, 
                   TrapControl& trapControl, IrrigationControl& irrigationControl);
    void begin(const char* ssid, const char* pass);
    void begin();
    void handleClient(const SensorData& sensors, const String& rtcTime);
    void cleanupSessions();
    void initializeAuthentication(const char* username, const char* password);
    void handleLoginRequest(WiFiClient& client, const String& request, const SensorData& sensors, const String& rtcTime);

private:
    WiFiServer _server;
    PumpControl& _pumpControl;
    DoorControl& _doorControl;
    ValveControl& _valveControl;
    ClimateControl& _climateControl;
    TrapControl& _trapControl;
    IrrigationControl& _irrigationControl;

    void sendLoginPage(WiFiClient& client, bool showError);
    void sendMainPage(WiFiClient& client, const SensorData& sensors, const String& rtcTime, const String& newSessionId = "");
    void sendErrorPage(WiFiClient& client, int errorCode, const String& message);
    void sendSecurityHeaders(WiFiClient& client, const String& sessionId);
    void sendUnauthorizedResponse(WiFiClient& client);
    String parseAuthHeader(const String& request);
    String parseSessionCookie(const String& request);
    String parseRequestPath(const String& request);
    String parseFormData(const String& request);
    String parseFormField(const String& formData, const String& fieldName);
    bool isValidRequest(const String& request);
    void processControlCommands(const String& path);
    void printWifiStatus();

    // Helper functions for building the main page
    void sendMainPageHeader(WiFiClient& client, const String& newSessionId, const String& rtcTime);
    void sendClimateCard(WiFiClient& client, const SensorData& sensors);
    void sendSoilMoistureCard(WiFiClient& client, const SensorData& sensors);
    void sendIrrigationCard(WiFiClient& client);
    void sendFansCard(WiFiClient& client);
    void sendVentilationTrapCard(WiFiClient& client);
    void sendDoorsCard(WiFiClient& client);
    void sendMainPageFooter(WiFiClient& client);
    void renderButton(WiFiClient& client, const String& path, const String& label, const String& cssClass);
};

#endif // SOLARWEBSERVER_H