#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "config.h"
#include "displayPMS.h"
#include "tca9548a.h"

static Adafruit_SSD1306 displayPMS(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
static bool displayPMSReady = false;

static float lastPm1 = 0.0f;
static float lastPm25 = 0.0f;
static float lastPm10 = 0.0f;

static uint8_t pmsScreenIndex = 0;
static unsigned long lastPMSSwitch = 0;

static void drawPMSView(uint8_t index)
{
    tcaSelect(0);

    displayPMS.clearDisplay();
    displayPMS.setTextColor(SSD1306_WHITE);
    displayPMS.setTextSize(2);

    switch (index)
    {
    case 0:
        displayPMS.setCursor(0, 0);
        displayPMS.println("PM1   part");
        displayPMS.setCursor(0, 24);
        displayPMS.print(lastPm1, 1);
        displayPMS.println(" ug/m3");
        break;

    case 1:
        displayPMS.setCursor(0, 0);
        displayPMS.print("PM2.5 part");
        displayPMS.setCursor(0, 24);
        displayPMS.print(lastPm25, 1);
        displayPMS.println(" ug/m3");
        break;

    default:
        displayPMS.setCursor(0, 0);
        displayPMS.println("PM10  part");
        displayPMS.setCursor(0, 24);
        displayPMS.print(lastPm10, 1);
        displayPMS.println(" ug/m3");
        break;
    }

    displayPMS.display();
}

void startDisplayPMS()
{
    tcaSelect(0);

    if (!displayPMS.begin(SSD1306_SWITCHCAPVCC, DISPLAY_PMS_ADDRESS))
    {
        Serial.println("Display PMS not found");
        displayPMSReady = false;
        return;
    }

    displayPMSReady = true;
    lastPMSSwitch = millis();
    pmsScreenIndex = 0;
    drawPMSView(pmsScreenIndex);
}

void updateDisplayPMS(float pm1, float pm25, float pm10)
{

    if (!displayPMSReady)
    {
        return;
    }

    lastPm1 = pm1;
    lastPm25 = pm25;
    lastPm10 = pm10;

    if (millis() - lastPMSSwitch >= 10000)
    {
        lastPMSSwitch = millis();
        pmsScreenIndex = (pmsScreenIndex + 1) % 3;
    }

    drawPMSView(pmsScreenIndex);
}