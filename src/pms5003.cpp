#include <Arduino.h>
#include <PMS.h>

#include "config.h"
#include "pms5003.h"

// UART2 - ESP32
HardwareSerial pmsSerial(2);

// Library driver
static PMS pms(pmsSerial);
static PMS::DATA data;

static float pm1 = 0.0f;
static float pm25 = 0.0f;
static float pm10 = 0.0f;
static bool hasValidData = false;

void startPMS5003()
{
    pmsSerial.begin(
        9600,
        SERIAL_8N1,
        PMS_RX,
        PMS_TX);

    pmsSerial.setTimeout(1500);
    pms.sleep();

    Serial.println("PMS5003 started in sleep mode");
}

void wakePMS5003()
{
    while (pmsSerial.available() > 0)
    {
        pmsSerial.read();
    }

    pms.wakeUp();
}

void sleepPMS5003()
{
    pms.sleep();
}

bool updatePMS5003()
{
    bool updated = false;

    // PMS::read processes one UART byte per call; drain the buffer to complete frames.
    while (pmsSerial.available() > 0)
    {
        if (pms.read(data))
        {
            pm1 = data.PM_AE_UG_1_0;
            pm25 = data.PM_AE_UG_2_5;
            pm10 = data.PM_AE_UG_10_0;
            hasValidData = true;
            updated = true;
        }
    }

    return updated;
}

bool hasPMSData()
{
    return hasValidData;
}

float readPM1()
{
    return pm1;
}

float readPM25()
{
    return pm25;
}

float readPM10()
{
    return pm10;
}