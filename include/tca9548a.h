#ifndef TCA9548A_H
#define TCA9548A_H

#include <stdint.h>

void tcaSelect(uint8_t channel);

void scanTCAChannels();

#endif