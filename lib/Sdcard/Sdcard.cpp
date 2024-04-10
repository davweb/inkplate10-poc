#include <DebugLog.h>
#include "Sdcard.h"

extern Inkplate display;

// Callback for adding current date and time to SD file
void fileDateTime(uint16_t* date, uint16_t* time) {
    display.rtcGetRtcData();
    uint8_t year = display.rtcGetYear();
    uint8_t month = display.rtcGetMonth();
    uint8_t day = display.rtcGetDay();
    uint8_t hour = display.rtcGetHour();
    uint8_t minute = display.rtcGetMinute();
    uint8_t second = display.rtcGetSecond();
    LOG_TRACE("Setting file date time", year, month, day, hour, minute, second);
    *date = FAT_DATE(year, month, day);
    *time = FAT_TIME(hour, minute, second);
}


// Initialise the SD card, logging on failure
bool startSdCard() {
    SdFile::dateTimeCallback(fileDateTime);
    int16_t result = display.sdCardInit();

    if (result == 1) {
        LOG_INFO("SD card init successful");
        return true;
    }
    else {
        LOG_WARN("SD card init failed");
        return false;
    }
}

