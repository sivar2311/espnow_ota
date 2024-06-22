#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Updater.h>
#include <espnow.h>

#include "webPage.h"

AsyncWebServer server(80);

uint8_t receiver_mac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void sendBinary(const uint8_t* data, size_t len) {
    int error = esp_now_send(0, (u8*)data, len);
    if (error != 0) {
        Serial.printf("Error %d sending espnow packet\r\n", error);
    } else {
        Serial.printf("espnow packet sent: %d bytes\r\n", len);
    }
}

void sendText(const char* data) {
    sendBinary((const uint8_t*)data, strlen(data) + 1);
}

void sendData(uint8_t* data, size_t len) {
    while (len) {
        size_t bytesToSend = len > 250 ? 250 : len;
        len -= bytesToSend;
        sendBinary(data, bytesToSend);
        data += bytesToSend;
    }
}

void notFound(AsyncWebServerRequest* request) {
    request->send(404, "text/plain", "Not found");
}

void handleRoot(AsyncWebServerRequest* request) {
    request->send(200, "text/html", webPage);
}

void handleSendUpdatePOST(AsyncWebServerRequest* request) {
    request->send(200, "text/plain", "Upload finished");
}

void handleSendUpdateFileupload(AsyncWebServerRequest* request, const String& filename, size_t index, uint8_t* data, size_t len, bool final) {
    if (index == 0) {
        Serial.printf("Uploading \"%s\"\r\n", filename.c_str());
        Serial.println("BEGINUPDATE");
        sendText("BEGINUPDATE");
    }

    sendData(data, len);

    if (final) {
        Serial.println("ENDUPDATE");
        sendText("ENDUPDATE");
    }
}

void setupAccessPoint() {
    WiFi.softAP("ESPNOW-SENDER");
    Serial.printf("Connect to wifi-network \"ESPNOW-SENDER\" and open http://%s/ in your browser\r\n", WiFi.softAPIP().toString().c_str());
}

void setupWebServer() {
    server.on("/", HTTP_GET, handleRoot);
    server.on("/send_update", HTTP_POST, handleSendUpdatePOST, handleSendUpdateFileupload);
    server.onNotFound(notFound);
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    server.begin();
}

void setupESPNOW() {
    int error = esp_now_init();

    if (error == 0) {
        Serial.println("ESP-NOW started");
    } else {
        Serial.println("Starting ESP-NOW failed!");
        return;
    }

    esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
    esp_now_add_peer(receiver_mac, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}

void setup() {
    Serial.begin(115200);
    Serial.println();
    setupAccessPoint();
    setupWebServer();
    setupESPNOW();
}

void loop() {
}