#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncMqttClient.h>
#include <EEPROM.h>
#include <Ticker.h>
#include <WiFi.h>
#include <WS2812FX.h>

#include "configuration.h"
#include "version.h"
#include "ota.h"

#include <map>
#include <string>

using namespace std;

#ifndef MAIN_h
#define MAIN_h

#define CMD_ON "ON"
#define CMD_DEAD "offline"
#define CMD_ALIVE "online"

struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct State {
    Color color = { 255, 255, 255 };
    uint8_t brightness = 100;
    string effect = "static";
    uint16_t speed = 1000; 
    bool on = true;
};

std::map<string, int> effects = {
    { "static", FX_MODE_STATIC },
    { "blink", FX_MODE_BLINK },
    { "breath", FX_MODE_BREATH },
    { "color wipe", FX_MODE_COLOR_WIPE },
    { "color wipe inverted", FX_MODE_COLOR_WIPE_INV },
    { "color wipe reverse", FX_MODE_COLOR_WIPE_REV },
    { "color wipe reverse inverted", FX_MODE_COLOR_WIPE_REV_INV },
    { "color wipe random", FX_MODE_COLOR_WIPE_RANDOM },
    { "random color", FX_MODE_RANDOM_COLOR },
    { "single dynamic", FX_MODE_SINGLE_DYNAMIC },
    { "multi dynamic", FX_MODE_MULTI_DYNAMIC },
    { "rainbow", FX_MODE_RAINBOW },
    { "rainbow cycle", FX_MODE_RAINBOW_CYCLE },
    { "scan", FX_MODE_SCAN },
    { "dual scan", FX_MODE_DUAL_SCAN },
    { "fade", FX_MODE_FADE },
    { "theater chase", FX_MODE_THEATER_CHASE },
    { "theater chase rainbow", FX_MODE_THEATER_CHASE_RAINBOW },
    { "running lights", FX_MODE_RUNNING_LIGHTS },
    { "twinkle", FX_MODE_TWINKLE },
    { "twinkle random", FX_MODE_TWINKLE_RANDOM },
    { "twinkle fade", FX_MODE_TWINKLE_FADE },
    { "twinkle fade random", FX_MODE_TWINKLE_FADE_RANDOM },
    { "sparkle", FX_MODE_SPARKLE },
    { "flash sparkle", FX_MODE_FLASH_SPARKLE },
    { "hyper sparkle", FX_MODE_HYPER_SPARKLE },
    { "strobe", FX_MODE_STROBE },
    { "strobe rainbow", FX_MODE_STROBE_RAINBOW },
    { "multi strobe", FX_MODE_MULTI_STROBE },
    { "blink rainbow", FX_MODE_BLINK_RAINBOW },
    { "chase white", FX_MODE_CHASE_WHITE },
    { "chase color", FX_MODE_CHASE_COLOR },
    { "chase random", FX_MODE_CHASE_RANDOM },
    { "chase rainbow", FX_MODE_CHASE_RAINBOW },
    { "chase flash", FX_MODE_CHASE_FLASH },
    { "chase random", FX_MODE_CHASE_RANDOM },
    { "chase rainbow white", FX_MODE_CHASE_RAINBOW_WHITE },
    { "chase blackout", FX_MODE_CHASE_BLACKOUT },
    { "chase blackout rainbow", FX_MODE_CHASE_BLACKOUT_RAINBOW },
    { "color sweep random", FX_MODE_COLOR_SWEEP_RANDOM },
    { "running color", FX_MODE_RUNNING_COLOR },
    { "running red blue", FX_MODE_RUNNING_RED_BLUE },
    { "running random", FX_MODE_RUNNING_RANDOM },
    { "larson scanner", FX_MODE_LARSON_SCANNER },
    { "comet", FX_MODE_COMET },
    { "fireworks", FX_MODE_FIREWORKS },
    { "fireworks random", FX_MODE_FIREWORKS_RANDOM },
    { "merry christmas", FX_MODE_MERRY_CHRISTMAS },
    { "fire flicker", FX_MODE_FIRE_FLICKER },
    { "fire flicker soft", FX_MODE_FIRE_FLICKER_SOFT },
    { "fire flicker intense", FX_MODE_FIRE_FLICKER_INTENSE },
    { "circus combustus", FX_MODE_CIRCUS_COMBUSTUS },
    { "halloween", FX_MODE_HALLOWEEN },
    { "bicolor chase", FX_MODE_BICOLOR_CHASE },
    { "tricolor chase", FX_MODE_TRICOLOR_CHASE },
    { "icu", FX_MODE_ICU }
};

AsyncMqttClient mqttClient;
WS2812FX ledstrip = WS2812FX(LED_COUNT, LED_PIN, LED_TYPE);
Ticker mqttReconnectTimer;
Ticker wifiReconnectTimer;
State currentState;

//Connection starters
void connectToWiFi();
void connectToMQTT();

// Event handlers
void onWiFiEvent(WiFiEvent_t event);
void onMQTTConnect(bool sessionPresent);
void onMQTTDisconnect(AsyncMqttClientDisconnectReason reason);
void onMQTTMessage(
    char* topic,
    char* payload,
    AsyncMqttClientMessageProperties properties,
    size_t len,
    size_t index,
    size_t total
);

// Helpers
void announceToHomeAssistant();
void processStateJson(char* payload);
void updateState();

// Main program functions
void setup();
void loop();

#endif
