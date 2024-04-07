#include <Inkplate.h>
#include <rom/rtc.h>
#include <DebugLog.h>
#include "Sleep.h"

#define uS_TO_S_FACTOR 1000000 // Conversion factor for seconds to micro seconds

extern Inkplate display;

WAKE_REASON getWakeReason() {
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

    switch (wakeup_reason) {
        // Woken from sleep by timer
        case ESP_SLEEP_WAKEUP_TIMER:
            LOG_DEBUG("Woken up by timer");
            return ALARM;
        // Woken from sleep by button
        case ESP_SLEEP_WAKEUP_EXT0:
            LOG_DEBUG("Woken up by button");
            return BUTTON;
        // Was not sleeping
        default:
            LOG_DEBUG("Not sleeping");
            return NOT_SLEEPING;
    }
}

void deepSleep(uint16_t seconds) {
    LOG_INFO("Going to sleep for", seconds, "seconds");

    esp_sleep_enable_timer_wakeup(seconds * uS_TO_S_FACTOR);

    // Wake up on button
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_36, LOW);

    esp_deep_sleep_start();
}