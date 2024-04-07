#include <Inkplate.h>
#include <DebugLog.h>
#include <ArduinoJson.h>
#include "ClockTools.h"
#include "HttpTools.h"

extern Inkplate display;

// Fetch time from NTP server and set the RTC
bool setRtcClock() {
    long epoch;
    if (!display.getNTPEpoch(&epoch)) {
        LOG_ERROR("Failed to get time from NTP server");
        return false;
    }

    JsonDocument doc;
    int offset;

    if (!getJsonFromUrl(doc, "http://ip-api.com/json/?fields=status,timezone,offset")) {
        LOG_WARN("Failed to get timezone from IP API. Using UTC.");
    }
    else {
        String timezone = doc["timezone"];
        int offset = doc["offset"];
        LOG_INFO("Timezone", timezone, "with offset", offset);
        epoch += offset;
    }

    display.rtcSetEpoch(epoch);
    char dateTime[20];
    getCurrentDateTime(dateTime);
    LOG_INFO("RTC clock set to", dateTime);
    return true;
}

// Get the current time and store in a state variable
void getCurrentTime(char* currentTime) {
    display.rtcGetRtcData();
    sprintf(currentTime, "%02d:%02d",
        display.rtcGetHour(),
        display.rtcGetMinute());
    LOG_TRACE("Current time", currentTime);
}
void getCurrentDateTime(char* currentDateTime) {
    display.rtcGetRtcData();
    sprintf(currentDateTime,
            "%4d-%02d-%02d %02d:%02d:%02d",
            display.rtcGetYear(),
            display.rtcGetMonth(),
            display.rtcGetDay(),
            display.rtcGetHour(),
            display.rtcGetMinute(),
            display.rtcGetSecond());
    LOG_TRACE("Current date time", currentDateTime);
}