typedef enum {
    NOT_SLEEPING,
    ALARM,
    BUTTON
} WAKE_REASON;

WAKE_REASON getWakeReason();
void deepSleep(uint16_t seconds);
