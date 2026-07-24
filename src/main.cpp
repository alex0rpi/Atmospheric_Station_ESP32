#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <esp_sleep.h>

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
#include "timeManager.h"

static float temperature = 0.0f;
static float humidity = 0.0f;
static float pressure = 0.0f;
static float esp32Temperature = 0.0f;
static float pm1 = 0.0f;
static float pm25 = 0.0f;
static float pm10 = 0.0f;
static bool networkServicesStarted = false;
static bool timeSynchronized = false;

static void sleepUntilMorning()
{
  const uint64_t sleepTimeUs = microsecondsUntilMorning();
  if (sleepTimeUs == 0)
    return;

  Serial.printf("Mode nocturn fins a les 08:00 (%lu segons)\n", static_cast<unsigned long>(sleepTimeUs / 1000000ULL));
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  esp_sleep_enable_timer_wakeup(sleepTimeUs);
  esp_deep_sleep_start();
}

static void startNightSleep()
{
  stopMQTT();
  networkServicesStarted = false;
  sleepPMS5003();
  sleepBME();
  sleepDisplayBME();
  sleepDisplayPMS();
  sleepUntilMorning();
}

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
    timeSynchronized = synchronizeTime();
    if (timeSynchronized)
    {
      if (isNightTime())
        sleepUntilMorning();
    }

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
  esp32Temperature = temperatureRead();

  pm1 = readPM1();
  pm25 = readPM25();
  pm10 = readPM10();

  updateDisplayBME(temperature, humidity, pressure);
  updateDisplayPMS(pm1, pm25, pm10);

  Serial.println("System ready.");
}

void loop()
{
  if (timeSynchronized)
  {
    if (isNightTime())
      startNightSleep();
  }

  if (networkServicesStarted)
  {
    handleOTA();
    maintainMQTT();
  }

  static unsigned long lastSensorUpdate = 0;
  static unsigned long lastDisplayUpdate = 0;
  static unsigned long lastPMSPoll = 0;
  static unsigned long pmsWakeStarted = 0;
  static unsigned long nextPMSWake = 0;
  static bool pmsAwake = false;
  static bool pmsMeasurementStarted = false;
  static bool pmsMeasurementUpdated = false;

  const unsigned long bme280SensorInterval = 15000;
  const unsigned long displayInterval = 15000;
  const unsigned long pmsReadInterval = 1000;
  const unsigned long pmsMeasurementInterval = 10UL * 60UL * 1000UL;
  const unsigned long pmsWarmupInterval = 30UL * 1000UL;

  unsigned long now = millis();

  if (!pmsAwake && (!pmsMeasurementStarted || now - nextPMSWake >= pmsMeasurementInterval))
  {
    wakePMS5003();
    pmsAwake = true;
    pmsMeasurementStarted = true;
    pmsMeasurementUpdated = false;
    pmsWakeStarted = now;
    lastPMSPoll = now;
    Serial.println("PMS5003 waking up for measurement");
  }

  if (pmsAwake && now - lastPMSPoll >= pmsReadInterval)
  {
    lastPMSPoll = now;

    if (updatePMS5003())
    {
      pm1 = readPM1();
      pm25 = readPM25();
      pm10 = readPM10();
      pmsMeasurementUpdated = true;
    }
  }

  if (pmsAwake && now - pmsWakeStarted >= pmsWarmupInterval)
  {
    if (pmsMeasurementUpdated)
    {
      pm1 = readPM1();
      pm25 = readPM25();
      pm10 = readPM10();

      publishFloat(TOPIC_PM1, "PM1", pm1);
      publishFloat(TOPIC_PM25, "PM2.5", pm25);
      publishFloat(TOPIC_PM10, "PM10", pm10);
    }
    else
    {
      Serial.println("PMS5003 did not provide a valid frame");
    }

    sleepPMS5003();
    pmsAwake = false;
    nextPMSWake = pmsWakeStarted;
    Serial.println("PMS5003 sleeping");
  }

  if (now - lastSensorUpdate >= bme280SensorInterval)
  {

    lastSensorUpdate = now;

    temperature = readTemperature();
    humidity = readHumidity();
    pressure = readPressure();
    esp32Temperature = temperatureRead();

    publishFloat(TOPIC_TEMPERATURE, "Temperature", temperature);
    publishFloat(TOPIC_HUMIDITY, "Humidity", humidity);
    publishFloat(TOPIC_PRESSURE, "Pressure", pressure);
    publishFloat(TOPIC_ESP32_TEMPERATURE, "ESP32 temperature", esp32Temperature);
  }

  // Refresh displays

  if (now - lastDisplayUpdate >= displayInterval)
  {
    lastDisplayUpdate = now;

    updateDisplayBME(temperature, humidity, pressure);

    updateDisplayPMS(pm1, pm25, pm10);
  }
}
