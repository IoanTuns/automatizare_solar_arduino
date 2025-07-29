#ifndef SOLAR_WEBSERVER_H
#define SOLAR_WEBSERVER_H

#include <WiFiS3.h>
#include "PumpControl.h"
#include "DoorControl.h"
#include "ValveControl.h"
#include "ClimateControl.h"
#include "TrapControl.h"
#include "IrrigationControl.h"
#include "SensorData.h"
#include <DHT.h>
#include "config.h"

/**
 * @class SolarWebServer
 * @brief A class to manage a web server for the garden automation system.
 */
class SolarWebServer {
public:
    /**
     * @brief Constructs a SolarWebServer object with a specified port.
     * @param port The port number on which the web server will listen. Defaults to WEB_SERVER_PORT.
     */
    SolarWebServer(uint16_t port, PumpControl& pumpControl, DoorControl& doorControl, ClimateControl& climateControl, ValveControl& valveControl, TrapControl& trapControl, IrrigationControl& irrigationControl);

    /**
     * @brief Starts the web server and connects to a WiFi network.
     * @param ssid The SSID of the WiFi network to connect to.
     * @param pass The password for the WiFi network.
     * This method first connects to the specified WiFi network and then starts the server.
     */
    void begin(const char* ssid, const char* pass);

    /**
     * @brief Handles incoming client requests.
     * @param sensors A struct containing all current sensor readings.
     * @param rtcTime A formatted string of the current time.
     * This method processes client requests and sends an HTML page with sensor data.
     */
    void handleClient(const SensorData& sensors, const String& rtcTime);

    void printWifiStatus();  // Declare the printWifiStatus method here

private:
    WiFiServer _server; ///< The WiFi server instance.
    PumpControl& _pumpControl;
    DoorControl& _doorControl;
    ValveControl& _valveControl;
    ClimateControl& _climateControl;
    TrapControl& _trapControl;
    IrrigationControl& _irrigationControl;
};

#endif // SOLAR_WEBSERVER_H