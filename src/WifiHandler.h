#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <string>
#include <vector>

class WifiHandler
{
private:
    const char *_ssid;
    const char *_pass;

    TaskHandle_t _taskHandle;
    static void _staticTaskFunc(void *pvParam);
    void _taskFunc();

public:
    WifiHandler(const char *ssid, const char *pass);
    ~WifiHandler();

    void checkStatus();
    void init();
};
