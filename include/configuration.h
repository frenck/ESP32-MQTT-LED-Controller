#ifndef CONFIGURATION_h
#define CONFIGURATION_h

// There are not many reasons to change this
#define MONITOR_BAUDRATE 115200

// Name as it initially pops up in Home Assistant
#define NAME "ledstrip"

// WiFi Connection information
#define WIFI_SSID "WiFi name"
#define WIFI_PASSWORD "WiFi password"
#define WIFI_HOSTNAME "ledstrip"

// OTA setup
const uint16_t OTA_PORT = 8266;
const char *OTA_PASSWORD = "ota password";

// MQTT Server information
#define MQTT_HOST "192.168.1.2"
#define MQTT_USERNAME "MQTT username"
#define MQTT_PASSWORD "MQTT password"
#define MQTT_PORT 1883

// MQTT topics used by this program
#define MQTT_TOPIC_STATE "ledstrip/state"
#define MQTT_TOPIC_COMMAND "ledstrip/set"
#define MQTT_TOPIC_AVAILABLE "ledstrip/status"

// LED SETUP
#define LED_PIN 13 // PIN number used on the ESP32
#define LED_COUNT 7 // Number of LED your strip has

// You might have a GRB or RGB strip, play around to test
// Most strips are 800Khz, some older/classic are 400Khz (like the WS2811)
#define LED_TYPE NEO_GRB + NEO_KHZ800
//#define LED_TYPE NEO_RGB + NEO_KHZ800
//#define LED_TYPE NEO_GRB + NEO_KHZ400
//#define LED_TYPE NEO_RGB + NEO_KHZ400

#endif