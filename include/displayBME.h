#ifndef DISPLAY_BME_H
#define DISPLAY_BME_H

void enableDisplayBME();
void disableDisplayBME();
void wakeDisplayBME();
void updateDisplayBME(float temperature, float humidity, float pressure);

#endif