This is a learning exercice now to to use Arduino and AI tools on unknown technologies.
Web server is up and running and all the cotrolers behave as expected.
Config file provide a comprehensive connectors and configurations.

Create and src/secrets.h with following details
`#ifndef SECRETS_H
#define SECRETS_H
// secrets.h - Contains sensitive information such as WiFi credentials and web authentication details.
#define WIFI_SSID "ssid"
#define WIFI_PASS "pass"

// Default web authentication credentials
// Not implemted but validated
#define DEFAULT_WEB_USERNAME "admin"
#define DEFAULT_WEB_PASSWORD "greenhouse123"

#endif // SECRETS_H`
