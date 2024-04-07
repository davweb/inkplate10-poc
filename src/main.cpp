#include <Inkplate.h>
#include <DebugLog.h>
#include <Sleep.h>
#include <Fonts.h>
#include <Sdcard.h>
#include <ClockTools.h>
#include <HttpTools.h>

#define REFRESH_COUNT 10 // How often we do a full refresh

void boot();
bool connectWiFi();
bool disconnectWiFi();
bool getStaticState();
void renderTime(bool black);
void displayValue(const String &label, const String &value);
void logBatteryLevel();
void initialise(const String &label, bool(&func)());
void renderState();
void getState();


// We store state in RTC memory so it persists across deep sleep and can be used
// to populate screen buffer
RTC_DATA_ATTR bool booted = false;
RTC_DATA_ATTR bool sdCardOk = false;
RTC_DATA_ATTR bool wiFiConnected = false;
RTC_DATA_ATTR int8_t temperature = 0;
RTC_DATA_ATTR double batteryVoltage = 0;
RTC_DATA_ATTR char macAddress[18] = { 0 };
RTC_DATA_ATTR char localIpAddress[16] = { 0 };
RTC_DATA_ATTR char publicIpAddress[16] = { 0 };
RTC_DATA_ATTR char location[40] = { 0 };
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

    switch (getWakeReason())
    {
        // Woken from sleep by timer
        case ALARM:
            count += 1;

            if (count == REFRESH_COUNT) {
                count = 0;
            }

            break;
        // Woken from sleep by button
        case BUTTON:
            count = 0;
            break;
        // Was not sleeping
        case NOT_SLEEPING:
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
        getCurrentTime(currentTime);

        // Redraw the whole screen with up to date values
        display.clearDisplay();
        renderState();
        renderTime(true);
        display.display();
    }
    else {
        // Redraw the whole screen with values from before sleep and load it in the buffer
        display.clearDisplay();
        renderState();
        renderTime(true);
        display.preloadScreen();

        // Wipe old time and write new time
        renderTime(false);
        getCurrentTime(currentTime);
        renderTime(true);

        // Force a partial update
        display.partialUpdate(true);
    }

    // Sleep until the top of the next minute
    display.rtcGetRtcData();
    uint8_t second = display.rtcGetSecond();
    uint16_t timeToSleepSeconds = 60 - second;
    deepSleep(timeToSleepSeconds);
}

// Main loop
void loop() {
    // We will never loop as the Inkplate will go to sleep
}

// Perform one time initialisation
void boot() {
    LOG_DEBUG("Booting...");

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
        displayValue("Local IP address", localIpAddress);
        displayValue("Public IP address", publicIpAddress);
        displayValue("Location", location);
    }
}

// Update the state of the Inkplate
void getState() {
    connectWiFi();

    sdCardOk = display.getSdCardOk();
    temperature = display.readTemperature();
    batteryVoltage = display.readBattery();
    wiFiConnected = display.isConnected();

    if (wiFiConnected) {
        WiFi.localIP().toString().toCharArray(localIpAddress, 16);

        JsonDocument doc;

        if (getJsonFromUrl(doc, "http://ip-api.com/json/?fields=status,query,city,country")) {
            String status = doc["status"];

            if (status == "success") {
                String query = doc["query"];
                query.toCharArray(publicIpAddress, 16);
                LOG_TRACE("Public IP address", publicIpAddress);

                // Weird bug when I used String instead of const char* in sprintf
                const char* city = doc["city"];
                const char* country = doc["country"];
                sprintf(location, "%s, %s", city, country);
                LOG_TRACE("Location", location);
            }
            else {
                LOG_WARN("query IP address API failed");
                publicIpAddress[0] = '\0';
                location[0] = '\0';
            }
        }
        else {
            LOG_WARN("Failed to query IP address API");
            publicIpAddress[0] = '\0';
            location[0] = '\0';
        }
    }
    else {
        localIpAddress[0] = '\0';
        publicIpAddress[0] = '\0';
    }

    disconnectWiFi();
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
        LOG_WARN("No WiFi credentials provided");
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
        char dataToWrite[25];
        char dateTime[20];
        getCurrentDateTime(dateTime);

        sprintf(dataToWrite,
            "%s,%.2f",
            dateTime,
            display.readBattery());

        LOG_TRACE("Battery level", dataToWrite);

        if (!display.getSdCardOk()) {
            LOG_WARN("SD card not available");
            return;
        }

        const char* fileName = BATTERY_LOG_FILE;

        if (!file.open(fileName, FILE_WRITE))
        {
            LOG_WARN("Error writing battery level to file");
        }
        else
        {
            file.write(dataToWrite);
            file.write("\n");
            file.close();
        }
    }
#endif



