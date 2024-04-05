#include <Inkplate.h>
#include <rom/rtc.h>
#include <Roboto_Regular16pt7b.h>
#include <Roboto_Bold16pt7b.h>
#include <Roboto_Bold24pt7b.h>
#include <RobotoMono_VariableFont_wght20pt7b.h>

#define uS_TO_S_FACTOR 1000000 // Conversion factor for seconds to micro seconds
#define REFRESH_COUNT 10 // How often we do a full refresh

void boot();
bool connectWiFi();
bool disconnectWiFi();
bool setRtcClock();
bool startSdCard();
bool getStaticState();
void renderTime(bool black);
void displayValue(const String &label, const String &value);
void logBatteryLevel();
void initialise(const String &label, bool(&func)());
void getCurrentTime();
void renderState();
void getState();
void fileDateTime(uint16_t* date, uint16_t* time);

RTC_DATA_ATTR bool booted = false;
RTC_DATA_ATTR bool sdCardOk = false;
RTC_DATA_ATTR bool wiFiConnected = false;
RTC_DATA_ATTR int8_t temperature = 0;
RTC_DATA_ATTR double batteryVoltage = 0;
RTC_DATA_ATTR char macAddress[18] = { 0 };
RTC_DATA_ATTR char ipAddress[16] = { 0 };
RTC_DATA_ATTR char dimensions[10] = { 0 };
RTC_DATA_ATTR char currentTime[6] = { 0 };
RTC_DATA_ATTR int count = 0;

// Use black and white display so partial updates are possible
Inkplate display(INKPLATE_1BIT);

SdFile file;

// Count refreshes so we know when to do a full refresh

// Entry point
void setup() {
    Serial.begin(115200);
    display.begin();

    if (!booted) {
        boot();
    }


    esp_sleep_wakeup_cause_t wakeup_reason;
    wakeup_reason = esp_sleep_get_wakeup_cause();
    switch (wakeup_reason)
    {
        // Woken from sleep by timer
        case ESP_SLEEP_WAKEUP_TIMER:
            count += 1;

            if (count == REFRESH_COUNT) {
                count = 0;
            }

            break;
        // Woken from sleep by button
        case ESP_SLEEP_WAKEUP_EXT0:
            Serial.println("Wake up caused by button");
            count = 0;
            break;
        // Was not sleeping
        default:
            count = 0;
            break;
    }

    // Full update every 10 wakes and on start or button press
    if (count == 0) {
        // Log battery level if enabled
        #ifdef BATTERY_LOG_FILE
            startSdCard();
            logBatteryLevel();
        #endif

        // Update State
        getState();
        getCurrentTime();

        // Redraw the whole screen with up to date values
        display.clearDisplay();
        renderState();
        renderTime(true);
        display.display();
    }
    else {
        // Redraw the whole screen with values from before sleep and load it in the buffer
        renderState();
        renderTime(true);
        display.preloadScreen();

        // Wipe old time and write new time
        renderTime(false);
        getCurrentTime();
        renderTime(true);

        // Force a partial update
        display.partialUpdate(true);
    }

    // Sleep until the top of the next minute
    display.rtcGetRtcData();
    uint8_t second = display.rtcGetSecond();
    uint8_t timeToSleepSeconds = 60 - second;
    esp_sleep_enable_timer_wakeup(timeToSleepSeconds * uS_TO_S_FACTOR);

    // Wake up on button
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_36, LOW);

    esp_deep_sleep_start();
}

// Main loop
void loop() {
    // We will never loop as the Inkplate will go to sleep
}

// Perform one time initialisation
void boot() {
    Serial.println("Booting...");

    display.setFont(&RobotoMono_VariableFont_wght20pt7b);
    display.clearDisplay();
    display.println("\n Booting...");
    display.display();

    initialise("Getting properties", getStaticState);
    initialise("Connecting to WiFi", connectWiFi);
    initialise("Setting time", setRtcClock);
    initialise("Disconnecting WiFi", disconnectWiFi);

    display.partialUpdate();
    booted = true;
}

//initialise something by calling a function
void initialise(const String &label, bool(&func)()) {
    display.print(" ");
    display.print(label);
    display.print("... ");
    display.partialUpdate();
    bool result = func();
    display.println(result ? "Success" : "Failed");
    // We deliberately don't call a partial update here to limit updates
}

// Read properties of Inkplate that won't change
bool getStaticState() {
    sprintf(dimensions, "%dx%d", display.width(), display.height());
    uint8_t mac[6];
    WiFi.macAddress(mac);
    sprintf(macAddress, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return true;
}

// Initialise the SD card, logging on failure
bool startSdCard() {
    SdFile::dateTimeCallback(fileDateTime);
    int16_t result = display.sdCardInit();

    if (result == 1) {
        return true;
    }
    else {
        Serial.println("SD card init failed");
        return false;
    }
}

// Render the state of the Inkplate on the screen apart from the Time
void renderState() {
    display.setCursor(30, 50);
    displayValue("SD card available", sdCardOk ? "Yes" : "No");
    displayValue("Temperature", String(temperature) + "C");
    displayValue("Battery", String(batteryVoltage) + "V");
    displayValue("Screen dimensions", dimensions);
    displayValue("MAC address", macAddress);
    displayValue("Network connected", wiFiConnected ? "Yes" : "No");

    if (wiFiConnected) {
        displayValue("IP address", ipAddress);
    }
}

// Update the state of the Inkplate
void getState() {
    sdCardOk = display.getSdCardOk();
    temperature = display.readTemperature();
    batteryVoltage = display.readBattery();
    wiFiConnected = display.isConnected();

    if (wiFiConnected) {
        WiFi.localIP().toString().toCharArray(ipAddress, 16);
    }
    else {
        ipAddress[0] = '\0';
    }
}

// Display a label and value on the screen
void displayValue(const String &label, const String &value) {
    int16_t x = display.getCursorX();
    display.setFont(&Roboto_Bold16pt7b);
    display.print(label);
    display.print(": ");
    display.setFont(&Roboto_Regular16pt7b);
    display.println(value);
    display.setCursor(x, display.getCursorY());
}

// Connect the Wifi
bool connectWiFi() {
    #ifdef WIFI_SSID
        return display.connectWiFi(WIFI_SSID, WIFI_PASSWORD, 15);
    #else
        Serial.println("No WiFi credentials provided");
        return false;
    #endif
}

bool disconnectWiFi() {
    #ifdef WIFI_SSID
        if (!display.isConnected()) {
            return false;
        }
        else {
            display.disconnect();
            return true;
        }
    #else
        return false;
    #endif
}


// Fetch time from NTP server and set the RTC
bool setRtcClock() {
    long epoch;
    if (!display.getNTPEpoch(&epoch)) {
        Serial.println("Failed to get time from NTP server");
        return false;
    }

    display.rtcSetEpoch(epoch);
    return true;
}

// Get the current time and store in a state variable
void getCurrentTime() {
    display.rtcGetRtcData();
    uint8_t hour = display.rtcGetHour();
    uint8_t minute = display.rtcGetMinute();
    uint8_t second = display.rtcGetSecond();
    sprintf(currentTime, "%02d:%02d", hour, minute);
}

// Show the current time on the screen, optionally in white to clear previous value
void renderTime(bool black) {
    display.setTextColor(black ? BLACK : WHITE);
    display.setFont(&Roboto_Bold24pt7b);
    display.setCursor(1060, 50);
    display.print(currentTime);
}

#ifdef BATTERY_LOG_FILE
    // Log battery level
    void logBatteryLevel() {
        if (!display.getSdCardOk()) {
            Serial.println("SD card not available");
            return;
        }

        const char* fileName = BATTERY_LOG_FILE;
        char dataToWrite[26];
        display.rtcGetRtcData();

        sprintf(dataToWrite,
            "%4d-%02d-%02d %02d:%02d:%02d,%.2f\n",
            display.rtcGetYear(),
            display.rtcGetMonth(),
            display.rtcGetDay(),
            display.rtcGetHour(),
            display.rtcGetMinute(),
            display.rtcGetSecond(),
            display.readBattery());

        if (!file.open(fileName, FILE_WRITE))
        {
            Serial.print("Error while writing to file");
        }
        else
        {
            file.write(dataToWrite);
            file.close();
        }
    }
#endif


// Callback for adding current date and time to SD file
void fileDateTime(uint16_t* date, uint16_t* time) {
    display.rtcGetRtcData();
    byte year = display.rtcGetYear();
    byte month = display.rtcGetMonth();
    byte day = display.rtcGetDay();
    byte hour = display.rtcGetHour();
    byte minute = display.rtcGetMinute();
    byte second = display.rtcGetSecond();
    *date = FAT_DATE(year, month, day);
    *time = FAT_TIME(hour, minute, second);
}
