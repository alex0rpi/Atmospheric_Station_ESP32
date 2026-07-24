#include <Wire.h>
#include <Adafruit_BME280.h>

#include "config.h"
#include "bme.h"

Adafruit_BME280 bme;

void startBME()
{

    if (!bme.begin(0x76))
    {
        Serial.println("ERROR: BME280 not found");

        while (true)
        {
            delay(1000);
        }
    }

    Serial.println("BME280 started");
}

void sleepBME()
{
    bme.setSampling(Adafruit_BME280::MODE_SLEEP);
}

float readTemperature()
{
    return bme.readTemperature();
}

float readHumidity()
{
    return bme.readHumidity();
}

float readPressure()
{
    return bme.readPressure() / 100.0F; // Convert Pa to hPa
}