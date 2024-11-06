#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <time.h>
#include "Display_EPD_W21_spi.h"
#include "Display_EPD_W21.h"

// Credentials
const char* ssid = "YOURSSID";
const char* password = "YOURWIFIPASSWORD";
const char* image_url = "http://YOURURL.COM/output.h"; // URL to image.
const char* wakeHoursUrl = "http://YOURURL.COM/timer.txt"; // URL to wakehours txt file.

// Timezone and NTP server for CET/CEST
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;       // CET offset in seconds
const int daylightOffset_sec = 3600;   // Additional hour for CEST

// Define the image data array in SPIRAM
unsigned char* imageData = nullptr;
const int expectedImageSize = 384000;

// Dynamic wake hours array, supports up to 10 times
int wakeHours[10];
int numWakeHours = 0; // Actual count of parsed wake hours

// Retry sleep time on failure (e.g., 120 minutes = 2 hours)
const uint64_t retrySleepDuration = 120 * 60 * 1000000ULL; // 120 minutes in microseconds

// Function to connect to Wi-Fi with timeout
bool connectToWiFi() {
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    unsigned long startAttemptTime = millis();

    // Timeout after 10 seconds
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected to WiFi");
        return true;
    } else {
        Serial.println("\nFailed to connect to WiFi");
        return false;
    }
}

// Sync time with NTP server
void syncTime() {
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    struct tm timeinfo;

    if (getLocalTime(&timeinfo)) {
        Serial.println(&timeinfo, "Current time: %Y-%m-%d %H:%M:%S");
    } else {
        Serial.println("Failed to obtain time");
    }
}

// Fetch wake hours from a text file on a server
bool fetchWakeHours() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected.");
        return false;
    }

    HTTPClient http;
    http.begin(wakeHoursUrl);
    int httpCode = http.GET();
    bool success = false;

    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        numWakeHours = 0;

        // Parse the first line of the text file
        int start = 0;
        int commaIndex;

        while ((commaIndex = payload.indexOf(',', start)) > 0 && numWakeHours < 10) {
            wakeHours[numWakeHours++] = payload.substring(start, commaIndex).toInt();
            start = commaIndex + 1;
        }

        // Add the last number after the final comma
        if (numWakeHours < 10 && start < payload.length()) {
            wakeHours[numWakeHours++] = payload.substring(start).toInt();
        }

        success = (numWakeHours > 0); // Ensure we got at least one wake-up hour
        if (success) {
            Serial.print("Fetched wake hours: ");
            for (int i = 0; i < numWakeHours; i++) {
                Serial.print(wakeHours[i]);
                if (i < numWakeHours - 1) Serial.print(", ");
            }
            Serial.println();
        } else {
            Serial.println("Failed to parse wake hours correctly.");
        }
    } else {
        Serial.printf("HTTP request failed, error: %d\n", httpCode);
    }

    http.end();
    return success;
}

// Initialize the display hardware and clear it
void initializeDisplay() {
    Serial.println("Initializing display...");

    pinMode(A14, INPUT);  // BUSY gpio=13
    pinMode(A15, OUTPUT); // RES gpio=12
    pinMode(A16, OUTPUT); // DC gpio=14
    pinMode(A17, OUTPUT); // CS gpio=27

    SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
    SPI.begin();

    EPD_init_fast();       // Full screen refresh initialization
    //PIC_display_Clear();   // Clear screen (DISABLED!!)
    Serial.println("Refreshing image...");
}

// Calculate deep sleep duration to the nearest target wake-up time
uint64_t calculateSleepDuration() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time for sleep calculation");
        return 7200 * 1000000ULL; // Default to 2 hour sleep if time fails
    }

    int currentHour = timeinfo.tm_hour;
    int currentMinute = timeinfo.tm_min;
    int currentSecond = timeinfo.tm_sec;
    int currentSeconds = (currentHour * 3600) + (currentMinute * 60) + currentSecond;

    int minSecondsToSleep = 24 * 3600; // Start with maximum possible (24 hours)

    // Find the nearest wake-up time
    for (int i = 0; i < numWakeHours; i++) {
        int targetSeconds = (wakeHours[i] * 3600);
        int secondsToSleep;

        if (currentSeconds < targetSeconds) {
            secondsToSleep = targetSeconds - currentSeconds;
        } else {
            // Schedule for the next day if time has passed
            secondsToSleep = (24 * 3600) - currentSeconds + targetSeconds;
        }

        // Find the minimum sleep duration to the next target time
        if (secondsToSleep < minSecondsToSleep) {
            minSecondsToSleep = secondsToSleep;
        }
    }

    Serial.printf("Calculated sleep duration: %d seconds\n", minSecondsToSleep);
    return minSecondsToSleep * 1000000ULL; // Convert to microseconds for esp_sleep_enable_timer_wakeup
}

// Download and parse image data from text-based .h file
bool downloadImageAsText() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected.");
        return false;
    }

    HTTPClient http;
    http.begin(image_url);
    int httpCode = http.GET();
    bool success = false;

    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        int dataCount = 0;

        imageData = (unsigned char*)ps_malloc(expectedImageSize);
        if (!imageData) {
            Serial.println("Failed to allocate memory.");
            http.end();
            return false;
        }

        for (int i = 0; i < payload.length() - 1 && dataCount < expectedImageSize; i++) {
            if (isxdigit(payload[i]) && isxdigit(payload[i + 1])) {
                String hexValue = payload.substring(i, i + 2);
                imageData[dataCount++] = (unsigned char)strtol(hexValue.c_str(), nullptr, 16);
                i++;
            }
        }

        if (dataCount == expectedImageSize) {
            Serial.println("Image downloaded and parsed successfully.");
            Serial.printf("Data count: %d bytes\n", dataCount);
            success = true;
        } else {
            Serial.printf("Parsed size does not match expected: %d bytes\n", dataCount);
        }
    } else {
        Serial.printf("HTTP request failed, error: %d\n", httpCode);
    }

    http.end();
    return success;
}

void setup() {
    Serial.begin(115200);
    
    if (!connectToWiFi()) {
        Serial.println("Entering deep sleep to retry Wi-Fi connection...");
        esp_sleep_enable_timer_wakeup(retrySleepDuration);
        esp_deep_sleep_start();
    }

    syncTime();

    if (!fetchWakeHours()) {
        Serial.println("Failed to fetch wake hours, entering retry sleep...");
        esp_sleep_enable_timer_wakeup(retrySleepDuration);
        esp_deep_sleep_start();
    }

    if (!downloadImageAsText()) {
        Serial.println("Failed to download image, entering retry sleep...");
        esp_sleep_enable_timer_wakeup(retrySleepDuration);
        esp_deep_sleep_start();
    }

    initializeDisplay();

    if (imageData != nullptr) {
        EPD_init_fast();
        PIC_display_Clear();
        delay(1000);
        PIC_display(imageData);
        delay(1000);
        EPD_sleep();
        delay(1000);
    } else {
        Serial.println("Image data is not available.");
    }

    uint64_t sleepDuration = calculateSleepDuration();
    Serial.printf("Entering deep sleep for %llu microseconds...\n", sleepDuration);
    esp_sleep_enable_timer_wakeup(sleepDuration);
    esp_deep_sleep_start();
}

void loop() {
    // Empty loop since everything runs once in setup
}
