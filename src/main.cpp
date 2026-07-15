#include <Arduino.h>
#include <Wire.h>
#include <math.h>

#include "config.h"
#include "bme.h"
#include "displayBME.h"
#include "displayPMS.h"
#include "tca9548a.h"
#include "utils.h"
#include "pms5003.h"
#include "wifi.h"
#include "ota.h"
#include "mqtt.h"

static float temperature = 0.0f;
static float humidity = 0.0f;
static float pressure = 0.0f;
static float pm1 = 0.0f;
static float pm25 = 0.0f;
static float pm10 = 0.0f;
static bool networkServicesStarted = false;

void setup()
{
  Serial.begin(115200);

  Wire.begin(I2C_SDA, I2C_SCL);

  bool wifiConnected = startWiFi();
  if (!wifiConnected)
  {
    Serial.println("Continuant sense WiFi; revisar el log serie per al diagnòstic.");
  }
  else
  {
    startOTA();
    startMQTT();
    networkServicesStarted = true;
  }
  startBME();
  startPMS5003();
  scanTCAChannels();
  startDisplayBME();
  startDisplayPMS();

  temperature = readTemperature();
  humidity = readHumidity();
  pressure = readPressure();

  pm1 = readPM1();
  pm25 = readPM25();
  pm10 = readPM10();

  updateDisplayBME(temperature, humidity, pressure);
  updateDisplayPMS(pm1, pm25, pm10);

  Serial.println("System ready.");
}

void loop()
{
  if (networkServicesStarted)
  {
    handleOTA();
    maintainMQTT();
  }

  static unsigned long lastSensorUpdate = 0;
  static unsigned long lastDisplayUpdate = 0;
  static unsigned long lastPMSPoll = 0;

  const unsigned long sensorInterval = 10000;
  const unsigned long displayInterval = 2000;
  const unsigned long pmsPollInterval = 250;

  unsigned long now = millis();

  if (now - lastPMSPoll >= pmsPollInterval)
  {
    lastPMSPoll = now;

    if (updatePMS5003())
    {
      pm1 = readPM1();
      pm25 = readPM25();
      pm10 = readPM10();
    }
  }

  if (now - lastSensorUpdate >= sensorInterval)
  {

    lastSensorUpdate = now;

    temperature = readTemperature();
    humidity = readHumidity();
    pressure = readPressure();

    if (!hasPMSData())
    {
      Serial.println("PMS5003 waiting for valid frame...");
    }

    publishFloat(TOPIC_TEMPERATURE, "Temperature", temperature);
    publishFloat(TOPIC_HUMIDITY, "Humidity", humidity);
    // publishFloat(TOPIC_PRESSURE, "Pressure", pressure);

    publishFloat(TOPIC_PM1, "PM1", pm1);
    publishFloat(TOPIC_PM25, "PM2.5", pm25);
    publishFloat(TOPIC_PM10, "PM10", pm10);
  }

  // Refresh displays

  if (now - lastDisplayUpdate >= displayInterval)
  {
    lastDisplayUpdate = now;

    updateDisplayBME(temperature, humidity, pressure);

    updateDisplayPMS(pm1, pm25, pm10);
  }
}
