#include <Inkplate.h>
#include <Roboto_Regular16pt7b.h>
#include <Roboto_Bold16pt7b.h>
#include <Roboto_Bold24pt7b.h>
#include <RobotoMono_VariableFont_wght20pt7b.h>


bool connectWifi();
bool setTime();
bool startSdCard();
void fullUpdate();
void partialUpdate();
void showTime();
void displayValue(const String &label, const String &value);
void logBatteryLevel();
void initialise(const String &label, bool(&func)());


// Use black and white display so partial updates are possible
Inkplate display(INKPLATE_1BIT);

SdFile file;

// Count refreshes so we know when to do a full refresh
int count = 0;


// Entry point
void setup() {
    Serial.begin(115200);
    Serial.println("Starting...");
    Serial.flush();

    display.begin();
    display.setFont(&RobotoMono_VariableFont_wght20pt7b);
    display.println("\n Starting...");
    display.display();

    initialise("Starting SD card", startSdCard);
    initialise("Connecting to Wifi", connectWifi);
    initialise("Setting time", setTime);

    display.disconnect();
}

// Main loop
void loop() {
    Serial.printf("Count: %d\n", count);

    if (count == 0) {
        fullUpdate();
    }
    else {
        partialUpdate();
    }

    count += 1;

    if (count == 10) {
        count = 0;
    }

    display.rtcGetRtcData();
    uint8_t second = display.rtcGetSecond();
    delay((60 - second) * 1000);
}

//initialise something by calling a function
void initialise(const String &label, bool(&func)()) {
    display.print(" ");
    display.print(label);
    display.print("... ");
    display.partialUpdate();
    bool result = func();
    display.println(result ? "Success" : "Failed");
    display.partialUpdate();
}

bool startSdCard() {
    int16_t result = display.sdCardInit();

    if (result == 1) {
        return true;
    }
    else {
        Serial.println("SD card init failed");
        return false;
    }
}

// Fetch all data and do a full refresh of the display
void fullUpdate() {
    display.clearDisplay();
    display.setCursor(30, 50);
    displayValue("SD card available", display.getSdCardOk() ? "Yes" : "No");
    displayValue("Temperature", String(display.readTemperature()) + "C");
    displayValue("Battery", String(display.readBattery()) + "V");
    displayValue("Screen dimensions", String(display.width()) + "x" + String(display.height()));
    displayValue("MAC address", WiFi.macAddress());

    bool isConnected = display.isConnected();
    displayValue("Network connected", isConnected ? "Yes" : "No");

    if (isConnected) {
        displayValue("IP address", WiFi.localIP().toString());
    }

    showTime();
    display.display();

    #ifdef BATTERY_LOG_FILE
        if (display.getSdCardOk()) {
            logBatteryLevel();
        }
    #endif
}

// Do a partial update just update the time
void partialUpdate() {
    showTime();
    display.partialUpdate();
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
bool connectWifi() {
    #ifdef WIFI_SSID
        return display.connectWiFi(WIFI_SSID, WIFI_PASSWORD, 15, true);
    #else
        Serial.println("No WiFi credentials provided");
        return false;
    #endif
}

// Fetch time from NTP server and set the RTC
bool setTime() {
    long epoch;
    if (!display.getNTPEpoch(&epoch)) {
        Serial.println("Failed to get time from NTP server");
        return false;
    }

    Serial.print("Setting epoch to ");
    Serial.println(epoch);
    display.rtcSetEpoch(epoch);
    return true;
}

// Write text clearing the area first for partial refreshes
void displayTextRefresh(const String &text) {
    int16_t x = display.getCursorX();
    int16_t y = display.getCursorY();
    int16_t top, bottom;
    uint16_t width, height;
    display.getTextBounds(text, x, y, &top, &bottom, &width, &height);
    display.fillRect(top, bottom, width, height, 0);
    display.print(text);
    // TODO fix bug of new text being smaller than previous text
}

// Show the current time on the screen
void showTime() {
    display.rtcGetRtcData();
    uint8_t hour = display.rtcGetHour();
    uint8_t minute = display.rtcGetMinute();
    char buffer[5];
    sprintf(buffer, "%02d:%02d", hour, minute);

    display.setFont(&Roboto_Bold24pt7b);
    display.setCursor(1060, 50);
    displayTextRefresh(buffer);
}


#ifdef BATTERY_LOG_FILE
    // Log battery level
    void logBatteryLevel() {
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