#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include "secrets.h"

// WIFI --------------------

// MQTT --------------------

constexpr char MQTT_DISCOVERY_PREFIX[] = "homeassistant";

constexpr char MQTT_CLIENT_ID[] = "envstation";
constexpr char DEVICE_NAME[] = "envstation";
constexpr char DEVICE_FRIENDLY_NAME[] = "Environmental Station";

constexpr char TOPIC_TEMPERATURE[] = "envstation/bme280/temperature";
constexpr char TOPIC_HUMIDITY[] = "envstation/bme280/humidity";
constexpr char TOPIC_PRESSURE[] = "envstation/bme280/pressure";

constexpr char TOPIC_PM1[] = "envstation/pms5003/pm1";
constexpr char TOPIC_PM25[] = "envstation/pms5003/pm25";
constexpr char TOPIC_PM10[] = "envstation/pms5003/pm10";

// I2C ---------------------

constexpr int I2C_SDA = 21;
constexpr int I2C_SCL = 22;

// OLED --------------------

constexpr uint8_t DISPLAY_BME_ADDRESS = 0x3C;
constexpr uint8_t DISPLAY_PMS_ADDRESS = 0x3C;

constexpr int OLED_WIDTH = 128;
constexpr int OLED_HEIGHT = 64;

// UART PMS5003 ------------

constexpr int PMS_RX = 16;
constexpr int PMS_TX = 17;

// Multiplexor
constexpr uint8_t TCA9548A_ADDRESS = 0x70;

#endif