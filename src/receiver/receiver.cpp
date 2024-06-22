#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Updater.h>
#include <espnow.h>

void handleUpdate(u8 *data, size_t len) {
    static bool update_running = false;

    if (!update_running && strcmp((const char *)data, "BEGINUPDATE") == 0) {
        Serial.println("Update started");
        update_running          = true;
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        Update.runAsync(true);
        Update.begin(maxSketchSpace, U_FLASH);
        return;
    }

    if (!update_running) return;

    if (len == 9) Serial.println((const char *)data);

    if (update_running && strcmp((const char *)data, "ENDUPDATE") == 0) {
        update_running = false;
        Serial.println("Update finished");
        if (!Update.end(true)) {
            Serial.print("Update Error: ");
            Update.printError(Serial);
        } else {
            Serial.println("Update successful!");
            delay(1000);
            ESP.reset();
        }
    } else {
        Serial.printf("Update: %d bytes\r\n", len);
        Update.write(data, len);
    }
}

void onReceive(u8 *mac_addr, u8 *data, u8 len) {
    handleUpdate(data, len);
}

void setupESPNOW() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    int error = esp_now_init();

    if (error == 0) {
        Serial.println("ESP-NOW started");
    } else {
        Serial.println("Starting ESP-NOW failed!");
        return;
    }

    esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
    esp_now_register_recv_cb(onReceive);
}

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("Receiver");
    Serial.println(WiFi.macAddress());

    setupESPNOW();
}

void loop() {
}