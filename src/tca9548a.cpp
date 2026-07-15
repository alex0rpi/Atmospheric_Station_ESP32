#include <Wire.h>
#include <Arduino.h>

#include "tca9548a.h"
#include "config.h"

void tcaSelect(uint8_t channel)
{
    if (channel > 7)
        return;

    Wire.beginTransmission(TCA9548A_ADDRESS);
    Wire.write(1 << channel);
    Wire.endTransmission();

    // Small margin to allow channel switching.
    delay(2);
}

void scanTCAChannels()
{
    Serial.println("Scanning TCA channels for I2C devices...");

    for (uint8_t ch = 0; ch < 8; ch++)
    {
        tcaSelect(ch);

        Serial.print("Channel ");
        Serial.print(ch);
        Serial.print(":");

        bool found = false;

        for (uint8_t addr = 1; addr < 127; addr++)
        {
            Wire.beginTransmission(addr);

            if (Wire.endTransmission() == 0)
            {
                Serial.print(" 0x");

                if (addr < 16)
                    Serial.print('0');

                Serial.print(addr, HEX);
                found = true;
            }
        }

        if (!found)
            Serial.print(" none");

        Serial.println();
    }

    Serial.println("TCA scan complete");
}