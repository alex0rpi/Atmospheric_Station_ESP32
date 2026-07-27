#include "timeManager.h"
#include "config.h"
#include <Arduino.h>
#include <time.h>

bool synchronizeTime()
{
    configTzTime(TIMEZONE, NTP_SERVER_PRIMARY, NTP_SERVER_SECONDARY);

    tm localTime;
    if (!getLocalTime(&localTime, 10000))
    {
        Serial.println("No s'ha pogut sincronitzar l'hora NTP; es mantindra actiu.");
        return false;
    }

    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S %Z %z", &localTime);
    Serial.printf("Hora sincronitzada: %s\n", timestamp);
    return true;
}

bool isNightTime()
{
    tm localTime;
    if (!getLocalTime(&localTime))
        return false;

    return localTime.tm_hour >= NIGHT_SLEEP_START_HOUR || localTime.tm_hour < NIGHT_SLEEP_END_HOUR;
}

uint64_t microsecondsUntilMorning()
{
    tm localTime;
    if (!getLocalTime(&localTime))
        return 0;

    tm wakeTime = localTime;
    wakeTime.tm_hour = NIGHT_SLEEP_END_HOUR;
    wakeTime.tm_min = 0;
    wakeTime.tm_sec = 0;
    wakeTime.tm_isdst = -1;

    if (localTime.tm_hour >= NIGHT_SLEEP_START_HOUR)
        wakeTime.tm_mday += 1;

    const time_t now = mktime(&localTime);
    const time_t wakeTimestamp = mktime(&wakeTime);
    const double secondsToWake = difftime(wakeTimestamp, now);

    if (secondsToWake <= 0)
        return 0;

    return static_cast<uint64_t>(secondsToWake) * 1000000ULL;
}