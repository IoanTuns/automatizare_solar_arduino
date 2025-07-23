#ifndef SOLAR_WEBSERVER_H
#define SOLAR_WEBSERVER_H

#include <WiFiS3.h>
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
    SolarWebServer(uint16_t port = WEB_SERVER_PORT);

    /**
     * @brief Starts the web server and connects to a WiFi network.
     * @param ssid The SSID of the WiFi network to connect to.
     * @param pass The password for the WiFi network.
     * This method first connects to the specified WiFi network and then starts the server.
     */
    void begin(const char* ssid, const char* pass);

    /**
     * @brief Handles incoming client requests.
     * @param tInt Interior temperature reading.
     * @param hInt Interior humidity reading.
     * @param tExt Exterior temperature reading.
     * @param hExt Exterior humidity reading.
     * This method processes client requests and sends an HTML page with sensor data.
     */
    void handleClient(float tInt, float hInt, float tExt, float hExt, const int* soilMoisture);

    void printWifiStatus();  // Declare the printWifiStatus method here

private:
    WiFiServer _server; ///< The WiFi server instance.
};

#endif // SOLAR_WEBSERVER_H