#include <WiFi.h>
#include <PubSubClient.h>

#include "config.h"
#include "mqtt.h"
#include "operationMode.h"

WiFiClient espClient;               // Objecte que representa una connexió TCP/IP
PubSubClient mqttClient(espClient); // Li diem que el faci servir per parlar amb el bróker de missatges

static const unsigned long reconnectIntervalMs = 5000;
static unsigned long lastConnectionAttemptMs = 0;

// Payload builder util funcions

static void addParameterToPayload(String &payload, const char *key, const char *value)
{
    payload += "\"";
    payload += key;
    payload += "\":\"";
    payload += value;
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

// Callback function which triggers upon receiving a message from the MQTT broker
static void onMqttMessage(char *topic, byte *payload, unsigned int length)
{
    String msg;
    for (unsigned int i = 0; i < length; i++)
    {
        msg += (char)payload[i];
        // Ha de construir la paraula TOGGLE, que és la que envia Home Assistant quan es prem el botó del mode de funcionament
    }

    if (strcmp(topic, TOPIC_CMD_MODE) == 0 && msg == "TOGGLE")
    {
        toggleOperationMode();
        publish(TOPIC_OPERATION_MODE, getOperationModeString(), true);
    }
}

static bool connectMQTT()
{
    Serial.print("Connectant a MQTT");

    if (mqttClient.connect(MQTT_CLIENT_ID))
    {
        Serial.println(" OK");
        mqttClient.subscribe(TOPIC_CMD_MODE);
        publishHomeAssistantDiscovery();

        publish(TOPIC_OPERATION_MODE, getOperationModeString(), true);

        return true;
    }

    Serial.print(" error, estat MQTT: ");
    Serial.println(mqttClient.state());
    return false;
}

void startMQTT()
{
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    mqttClient.setCallback(onMqttMessage);
    lastConnectionAttemptMs = millis() - reconnectIntervalMs;
}

void stopMQTT()
{
    mqttClient.disconnect();
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
    auto publishButton = [](
                             const char *buttonName,
                             const char *buttonId,
                             const char *commandTopic,
                             const char *payloadPress)
    {
        String discoveryTopic = "homeassistant/button/" + String(DEVICE_NAME) + "/" + buttonId + "/config";
        String payload = "{";
        addParameterToPayload(payload, "name", buttonName);
        addParameterToPayload(payload, "command_topic", commandTopic);
        addParameterToPayload(payload, "payload_press", payloadPress);

        String uniqueId = String(DEVICE_NAME) + "_" + buttonId;
        addParameterToPayload(payload, "unique_id", uniqueId.c_str());

        addDeviceToPayload(payload);

        publish(discoveryTopic.c_str(), payload.c_str(), true);
    };
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

        String uniqueId = String(DEVICE_NAME) + "_" + sensorId;
        addParameterToPayload(payload, "unique_id", uniqueId.c_str());
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

    publishSensor("Operation Mode", "operation_mode", TOPIC_OPERATION_MODE, "", "", "");
    publishButton("Canviar mode", "mode_toggle", TOPIC_CMD_MODE, "TOGGLE");
}

/* Payload template example of a published Home Assistant discovery message for a sensor:
 *{
 *    "name": "Temperature",
 *    "state_topic": "envstation/bme280/temperature",
 *    "unit_of_measurement": "°C",
 *    "device_class": "temperature",
 *    "state_class": "measurement",
 *    "unique_id": "envstation_temperature",
 *    "device": {
 *        "identifiers": ["envstation"],
 *        "name": "Environmental Station",
 *        "manufacturer": "Espressif",
 *        "model": "ESP32 Environmental Station"
 *    }
 *}
 */
/* Payload template example of a published button
 *{
 *    "name": "Canviar mode",
 *    "command_topic": "envstation/cmd/mode",
 *    "payload_press": "TOGGLE",
 *    "unique_id": "envstation_mode_toggle",
 *    "device": {
 *        "identifiers": ["envstation"],
 *        "name": "Environmental Station",
 *        "manufacturer": "Espressif",
 *        "model": "ESP32 Environmental Station"
 *    }
 *}
 */