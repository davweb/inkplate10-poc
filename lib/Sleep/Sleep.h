typedef enum {
    NOT_SLEEPING,
    ALARM,
    BUTTON
} WAKE_REASON;

/**
 * Return the reason the device woke up from sleep
 *
 * @return The reason the device woke up from sleep
*/
WAKE_REASON getWakeReason();

/**
 * Put the device into deep sleep and wake after given number of seconds
 *
 * @param seconds The number of seconds to sleep
*/
void deepSleep(uint16_t seconds);
