#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WebServer.h>
#include <IPAddress.h>

struct user_data {
    const char* user;
    const char* pass;
};

const  user_data allowedUsers[] = {
    { "Jov",            "pass" },
    { "AirSense_Guest", "guest123" },
    { "p",              "getacc" },
    { "UserX",          "abcX" }
};

class DeviceServer {
    const char* ssid = "myESP32";
    const char* password = "esp32wifi";
    WebServer server;
    IPAddress IP;
    
    public:
    DeviceServer(uint16_t port);
    bool begin(const int Sensor_pin);
    void handle();
    
    const size_t allowedUserCount = sizeof(allowedUsers) / sizeof(allowedUsers[0]);
    
    ~DeviceServer();
};

#endif