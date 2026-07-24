#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "config.h"
#include "displayBME.h"
#include "tca9548a.h"

static Adafruit_SSD1306 displayBME(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
static bool displayBMEReady = false;

static float lastTemperature = 0.0f;
static float lastHumidity = 0.0f;
static float lastPressure = 0.0f;

static uint8_t bmeScreenIndex = 0;
static unsigned long lastBMESwitch = 0;

static void drawBMEView(uint8_t index)
{
    tcaSelect(1);

    displayBME.clearDisplay();
    displayBME.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    displayBME.setTextSize(2);

    const int valueY = 30;
    const int unitY = 46;

    switch (index)
    {
    case 0:
        displayBME.setCursor(0, 0);
        displayBME.println("TEMP");
        displayBME.setTextSize(3);
        displayBME.setCursor(0, valueY);
        displayBME.print(lastTemperature, 1);
        displayBME.setTextSize(2);
        displayBME.setCursor(116, unitY);
        displayBME.print("C");
        break;
    case 1:
        displayBME.setCursor(0, 0);
        displayBME.println("HUMITAT");
        displayBME.setTextSize(3);
        displayBME.setCursor(0, valueY);
        displayBME.print(lastHumidity, 1);
        displayBME.setTextSize(2);
        displayBME.setCursor(116, unitY);
        displayBME.print("%");
        break;
    default:
        displayBME.setCursor(0, 0);
        displayBME.println("PRESSIO");
        displayBME.setTextSize(3);
        displayBME.setCursor(0, valueY);
        displayBME.print(lastPressure, 0);
        displayBME.setTextSize(2);
        displayBME.setCursor(92, unitY);
        displayBME.print("hPa");
        break;
    }

    displayBME.display();
}

void startDisplayBME()
{
    tcaSelect(1);

    if (!displayBME.begin(SSD1306_SWITCHCAPVCC, DISPLAY_BME_ADDRESS))
    {
        Serial.println("Display BME not found");
        displayBMEReady = false;
        return;
    }

    // Force normal polarity: black background with illuminated pixels.
    displayBME.invertDisplay(false);
    displayBME.dim(false);

    displayBMEReady = true;
    lastBMESwitch = millis();
    bmeScreenIndex = 0;
    drawBMEView(bmeScreenIndex);
}

void sleepDisplayBME()
{
    if (!displayBMEReady)
        return;

    tcaSelect(1);
    displayBME.ssd1306_command(SSD1306_DISPLAYOFF);
}

void updateDisplayBME(float temperature, float humidity, float pressure)
{
    if (!displayBMEReady)
        return;

    lastTemperature = temperature;
    lastHumidity = humidity;
    lastPressure = pressure;

    if (millis() - lastBMESwitch >= 10000)
    {
        lastBMESwitch = millis();
        bmeScreenIndex = (bmeScreenIndex + 1) % 3;
    }

    drawBMEView(bmeScreenIndex);
}