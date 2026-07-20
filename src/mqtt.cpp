#include <WiFi.h>
#include <PubSubClient.h>

#include "config.h"
#include "mqtt.h"

WiFiClient espClient;               // Objecte que representa una connexió TCP/IP
PubSubClient mqttClient(espClient); // Li diem que el faci servir per parlar amb el broker de missatges

static const unsigned long reconnectIntervalMs = 5000;
static unsigned long lastConnectionAttemptMs = 0;

static void addParameterToPayload(String &payload, const char *key, const char *value)
{
    payload += "\"";
    payload += key;
    payload += "\":\"";
    payload += value;
    payload += "\",";
}

static void addSensorIdToPayload(String &payload, const char *sensorId)
{
    payload += "\"unique_id\":\"";
    payload += DEVICE_NAME;
    payload += "_";
    payload += sensorId;
    payload += "\",";
}

static void addDeviceToPayload(String &payload)
{
    payload += "\"device\":{";
    payload += "\"identifiers\":[\"";
    payload += DEVICE_NAME;
    payload += "\"],";
    payload += "\"name\":\"";
    payload += DEVICE_FRIENDLY_NAME;
    payload += "\",";
    payload += "\"manufacturer\":\"Espressif\",";
    payload += "\"model\":\"ESP32 Environmental Station\"";
    payload += "}}";
}

static bool connectMQTT()
{
    Serial.print("Connectant a MQTT");

    if (mqttClient.connect(MQTT_CLIENT_ID))
    {
        Serial.println(" OK");
        publishHomeAssistantDiscovery();
        return true;
    }

    Serial.print(" error, estat MQTT: ");
    Serial.println(mqttClient.state());
    return false;
}

void startMQTT()
{
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    lastConnectionAttemptMs = millis() - reconnectIntervalMs;
}

void maintainMQTT()
{
    if (!mqttClient.connected())
    {
        unsigned long now = millis();

        if (now - lastConnectionAttemptMs >= reconnectIntervalMs)
        {
            lastConnectionAttemptMs = now;
            connectMQTT();
        }

        return;
    }

    mqttClient.loop();
}

void publish(const char *topic, const char *mensaje)
{
    publish(topic, mensaje, false);
}

void publish(const char *topic, const char *mensaje, bool retained)
{
    mqttClient.publish(topic, mensaje, retained);
}

void publishHomeAssistantDiscovery()
{

    auto publishSensor = [](
                             const char *sensorName,
                             const char *sensorId,
                             const char *stateTopic,
                             const char *unit,
                             const char *deviceClass,
                             const char *stateClass)
    {
        String discoveryTopic = "homeassistant/sensor/" + String(DEVICE_NAME) + "/" + sensorId + "/config";
        String payload = "{";
        addParameterToPayload(payload, "name", sensorName);
        addParameterToPayload(payload, "state_topic", stateTopic);

        if (strlen(unit) > 0)
            addParameterToPayload(payload, "unit_of_measurement", unit);

        if (strlen(deviceClass) > 0)
            addParameterToPayload(payload, "device_class", deviceClass);

        if (strlen(stateClass) > 0)
            addParameterToPayload(payload, "state_class", stateClass);

        addSensorIdToPayload(payload, sensorId);
        addDeviceToPayload(payload);

        // Publiquem el topic a MQTT perquè Home Assistant el descobreixi automàticament
        publish(discoveryTopic.c_str(), payload.c_str(), true);
    };

    publishSensor("Temperature", "temperature", TOPIC_TEMPERATURE, "°C", "temperature", "measurement");
    publishSensor("Humidity", "humidity", TOPIC_HUMIDITY, "%", "humidity", "measurement");
    publishSensor("Pressure", "pressure", TOPIC_PRESSURE, "hPa", "pressure", "measurement");
    publishSensor("ESP32 Temperature", "esp32_temperature", TOPIC_ESP32_TEMPERATURE, "°C", "temperature", "measurement");

    publishSensor("PM1", "pm1", TOPIC_PM1, "µg/m³", "pm1", "measurement");
    publishSensor("PM2.5", "pm25", TOPIC_PM25, "µg/m³", "pm25", "measurement");
    publishSensor("PM10", "pm10", TOPIC_PM10, "µg/m³", "pm10", "measurement");
}