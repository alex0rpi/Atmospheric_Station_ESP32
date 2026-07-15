#include <WiFi.h>

#include "config.h"
#include "wifi.h"

static const char *wifiStatusToString(wl_status_t status)
{
    switch (status)
    {
    case WL_NO_SHIELD:
        return "no shield";
    case WL_IDLE_STATUS:
        return "idle";
    case WL_NO_SSID_AVAIL:
        return "ssid not found";
    case WL_SCAN_COMPLETED:
        return "scan completed";
    case WL_CONNECTED:
        return "connected";
    case WL_CONNECT_FAILED:
        return "connect failed";
    case WL_CONNECTION_LOST:
        return "connection lost";
    case WL_DISCONNECTED:
        return "disconnected";
    default:
        return "unknown";
    }
}

static void dumpVisibleNetworks(const char *targetSsid)
{
    int networkCount = WiFi.scanNetworks();

    if (networkCount <= 0)
    {
        Serial.println("No s'han trobat xarxes WiFi visibles.");
        return;
    }

    Serial.println("Xarxes visibles:");

    bool targetFound = false;

    for (int index = 0; index < networkCount; ++index)
    {
        String ssid = WiFi.SSID(index);

        Serial.printf(
            "  %2d: %s | RSSI %d dBm | canal %d | auth %d\n",
            index + 1,
            ssid.c_str(),
            WiFi.RSSI(index),
            WiFi.channel(index),
            static_cast<int>(WiFi.encryptionType(index)));

        if (ssid == targetSsid)
        {
            targetFound = true;
        }
    }

    if (!targetFound)
    {
        Serial.print("L'SSID objectiu no apareix a l'escaneig: ");
        Serial.println(targetSsid);
        Serial.println("Comprova que el router ofereix una xarxa 2.4 GHz amb aquest nom.");
    }

    WiFi.scanDelete();
}

bool startWiFi()
{
    Serial.print("Connectant a la WiFi");

    WiFi.persistent(false);
    WiFi.setAutoReconnect(true);
    WiFi.setSleep(false);

    const unsigned long timeoutMs = 30000;
    const unsigned long statusIntervalMs = 2000;
    const int maxAttempts = 3;

    for (int attempt = 1; attempt <= maxAttempts; ++attempt)
    {
        Serial.printf("\nIntent %d/%d amb SSID \"%s\"\n", attempt, maxAttempts, WIFI_SSID);

        WiFi.mode(WIFI_OFF);
        delay(200);
        WiFi.mode(WIFI_STA);
        WiFi.disconnect(false, true);
        delay(200);
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

        unsigned long startMs = millis();
        unsigned long lastStatusMs = 0;
        wl_status_t lastStatus = WiFi.status();

        while (lastStatus != WL_CONNECTED)
        {
            unsigned long now = millis();
            wl_status_t currentStatus = WiFi.status();

            if (currentStatus != lastStatus || now - lastStatusMs >= statusIntervalMs)
            {
                lastStatusMs = now;
                lastStatus = currentStatus;
                Serial.print("WiFi status: ");
                Serial.println(wifiStatusToString(currentStatus));
            }

            if (now - startMs >= timeoutMs)
            {
                Serial.println("WiFi timeout waiting for connection");
                Serial.print("Final status: ");
                Serial.println(wifiStatusToString(WiFi.status()));
                dumpVisibleNetworks(WIFI_SSID);
                break;
            }

            delay(100);
        }

        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println();
            Serial.println("WiFi connectat");
            Serial.print("adreca IP: ");
            Serial.println(WiFi.localIP());
            Serial.print("RSSI: ");
            Serial.println(WiFi.RSSI());
            return true;
        }
    }

    Serial.println("WiFi no s'ha connectat; el sistema continua en mode offline.");
    return false;
}