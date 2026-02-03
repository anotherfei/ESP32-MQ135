#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>

class wifi_manager {
    const char* ssid = "myESP32";
    const char* password = "esp32wifi";
    WiFiServer server;
    IPAddress IP;
public:
    wifi_manager(uint16_t port);
    bool begin();
    void manageClient(const int Sensor_pin);

    ~wifi_manager();
};

#endif