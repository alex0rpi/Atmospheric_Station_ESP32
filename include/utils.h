#ifndef UTILS_H
#define UTILS_H

void publishFloat(const char *topic, const char *label, float value);

float calculateAbsoluteHumidity(float temperature,
                                float relativeHumidity);

#endif