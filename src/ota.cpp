#include <Arduino.h>
#include <ArduinoOTA.h>
#include <WiFi.h>
#include <WiFi.h>
#include "secrets.h"
#include "ota.h"

void startOTA()
{
    ArduinoOTA.setHostname("envstation");

    // To protect OTA with password, uncomment:
    ArduinoOTA.setPassword(OTA_PASSWORD);

    ArduinoOTA.onStart([]()
                       { Serial.println("OTA start"); });

    ArduinoOTA.onEnd([]()
                     { Serial.println("\nOTA end"); });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                          { Serial.printf("OTA progress: %u%%\r", (progress * 100U) / total); });

    ArduinoOTA.onError([](ota_error_t error)
                       { Serial.printf("OTA error[%u]\n", error); });

    ArduinoOTA.begin();
    Serial.println("OTA ready");
    Serial.print("OTA IP: ");
    Serial.println(WiFi.localIP());
}

void handleOTA()
{
    ArduinoOTA.handle();
}