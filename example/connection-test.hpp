#pragma once

#include <Arduino.h>
#include "WifiHandler/src/WifiHandler.h"

WifiHandler wifi("eFisheryPlus", "123123123");

void setup()
{
    Serial.begin(115200);
    Serial.println("\nWiFi Handler Macca Lab Test !");
    vTaskDelay(1000);

    wifi.init();
}

void loop()
{
    static int count;
    Serial.printf("Count : %d\n", ++count);
    vTaskDelay(1000);
}