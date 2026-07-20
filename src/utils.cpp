#include <Arduino.h>

#include "mqtt.h"
#include <utils.h>

void publishFloat(const char *topic, const char *label, float value)
{
    char payload[16];

    dtostrf(value, 0, 1, payload);

    Serial.print(label);
    Serial.print(": ");
    Serial.println(payload);

    // Publish on MQTT topic
    publish(topic, payload);
}

float calculateAbsoluteHumidity(float temperature, float relativeHumidity)
{
    float saturationVaporPressure =
        6.112f * exp((17.62f * temperature) / (243.12f + temperature));

    float actualVaporPressure =
        (relativeHumidity / 100.0f) * saturationVaporPressure;

    return (216.7f * actualVaporPressure) / (273.15f + temperature);
}