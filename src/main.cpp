#include "main.h"

using namespace std;

void connectToWiFi()
{
    // FIX FOR USING 2.3.0 CORE (only .begin if not connected)
    if (WiFi.status() == WL_CONNECTED) return;

    Serial.println("WIFI: Connecting to WiFi...");
    WiFi.setHostname(WIFI_HOSTNAME);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void onWiFiEvent(WiFiEvent_t event)
{
    switch(event) {
        case SYSTEM_EVENT_STA_START:
            Serial.println("WIFI: Connecting...");
            break;
        case SYSTEM_EVENT_STA_CONNECTED:
            Serial.println("WIFI: Connected! Waiting for IP...");
            break;
        case SYSTEM_EVENT_STA_LOST_IP:
            Serial.println("WIFI: Lost IP address...");
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            Serial.println("WIFI: Got IP!");
            Serial.print("WIFI: IP Address: ");
            Serial.println(WiFi.localIP());
            connectToMQTT();
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            Serial.println("WIFI: Disconnected.");
            mqttReconnectTimer.detach();
            wifiReconnectTimer.once(2, connectToWiFi);
            break;
        default:
            break;
    }
}

void onMQTTConnect(bool sessionPresent)
{
    Serial.println("MQTT: Connected.");
    mqttClient.subscribe(MQTT_TOPIC_COMMAND, 0);
    mqttClient.publish(MQTT_TOPIC_AVAILABLE, 0, true, CMD_ALIVE);
    announceToHomeAssistant();
    updateState();
}

void connectToMQTT()
{
    Serial.println("MQTT: Connecting to broker...");
    mqttClient.setClientId(WIFI_HOSTNAME);
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    mqttClient.setCredentials(MQTT_USERNAME, MQTT_PASSWORD);
    mqttClient.setWill(MQTT_TOPIC_AVAILABLE, 0, true, CMD_DEAD);
    mqttClient.connect();
}

void onMQTTDisconnect(AsyncMqttClientDisconnectReason reason)
{
    Serial.println("MQTT: Disconnected.");
    if (WiFi.isConnected()) {
        mqttReconnectTimer.once(10, connectToMQTT);
    }
}

void onMQTTMessage(
    char* topic, char* payload,
    AsyncMqttClientMessageProperties properties,
    size_t len, size_t index, size_t total
) {
    Serial.println("MQTT: message received");
    Serial.println(payload);
    processStateJson(payload);
}

void processStateJson(char* payload) {
    StaticJsonBuffer<512> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(payload);

    if (!root.success()) {
        Serial.println("ParseObject() failed, skipping...");
        return;
    }

    if (root.containsKey("state") 
        && strcasecmp(root["state"].as<const char*>(), CMD_ON) == 0) {
        currentState.on = true;
    } else {
        currentState.on = false;
    }

    if (root.containsKey("speed")) {
        currentState.speed = root["speed"].as<uint16_t>();
    }

    if (root.containsKey("brightness")) {
        currentState.brightness = root["brightness"].as<uint8_t>();
    }

    if (root.containsKey("effect")) {
        string effect = root["effect"].as<const char*>();
        if (!effect.empty()) {
            currentState.effect = effect;
        }
    }

    if (root.containsKey("color")) {
        currentState.color.r = root["color"]["r"].as<uint8_t>();
        currentState.color.g = root["color"]["g"].as<uint8_t>();
        currentState.color.b = root["color"]["b"].as<uint8_t>();
    }

    updateState();
}

void announceToHomeAssistant()
{
    Serial.println("MQTT: Annoucing to Home Assistant");

    StaticJsonBuffer<2048> jsonObject;
    JsonObject& discovery = jsonObject.createObject();
    discovery.set("name", NAME);
    discovery.set("unique_id", WiFi.macAddress());
    discovery.set("platform", "mqtt_json");
    discovery.set("state_topic", MQTT_TOPIC_STATE);
    discovery.set("availability_topic", MQTT_TOPIC_AVAILABLE);
    discovery.set("command_topic", MQTT_TOPIC_COMMAND);
    discovery.set("brightness", true);
    discovery.set("rgb", true);
    discovery.set("effect", true);

    /// Adds effect list
    JsonArray& effectList = discovery.createNestedArray("effect_list");
    for(std::map<string,int>::iterator e = effects.begin(); e != effects.end(); ++e) {
         effectList.add(e->first.c_str());
    }

    char payload[2048];
    discovery.printTo(payload);
    Serial.println(payload);
    mqttClient.publish(
        "homeassistant/light/ledstrip/config",
        2,
        true,
        payload
    );
}

void updateState()
{
    Serial.println("STATE: Updating...");

    (currentState.on) ? ledstrip.start() : ledstrip.stop();
    ledstrip.setBrightness(currentState.brightness);
    ledstrip.setMode(effects[currentState.effect.c_str()]);
    ledstrip.setSpeed(currentState.speed);
    ledstrip.setColor(
        currentState.color.r,
        currentState.color.g,
        currentState.color.b
    );

    StaticJsonBuffer<512> currentJsonStateBuffer;
    JsonObject& currentJsonState = currentJsonStateBuffer.createObject();

    currentJsonState.set("speed", currentState.speed);
    currentJsonState.set("brightness", currentState.brightness);
    currentJsonState.set("state", currentState.on ? "ON" : "OFF");
    currentJsonState.set("effect", currentState.effect.c_str());

    JsonObject& currentJsonStateColor = currentJsonState.createNestedObject("color");
    currentJsonStateColor.set("r", currentState.color.r);
    currentJsonStateColor.set("g", currentState.color.g);
    currentJsonStateColor.set("b", currentState.color.b);

    char payload[512];
    currentJsonState.printTo(payload);

    // Store current state in EEPROM.
    Serial.println("EEPROM: Writing current state...");
    EEPROM.writeString(0, payload);
    EEPROM.commit();

    if (mqttClient.connected()) {
        Serial.println("STATE: Updating state on MQTT state topic");
        Serial.println(payload);
        mqttClient.publish(
            "ledstrip/state",
            2,
            true,
            payload
        );
    }
}

void setup()
{
    Serial.begin(MONITOR_BAUDRATE);
    Serial.println();
    Serial.println();

    // Small delay, for easier development
    // and give the CAPs a bit time to charge.
    delay(2000);

    Serial.println("LED Controller");
    Serial.printf("Version %s\n\n", VERSION);

    // Initialize EEPROM
    if (!EEPROM.begin(512)) {
        Serial.println("Failed to initialise EEPROM");
        Serial.println("Restarting...");
        delay(1000);
        ESP.restart();
    }

    // Setup LED strip
    ledstrip.init();

    // Read previous state from EEPROM
    Serial.println("EEPROM: Reading previous state...");
    String previousState = EEPROM.readString(0);
    if (previousState.length() > 1) {
        processStateJson(strdup(previousState.c_str()));
    } else {
        updateState();
    }

    // Register event handling
    WiFi.onEvent(onWiFiEvent);
    mqttClient.onConnect(onMQTTConnect);
    mqttClient.onDisconnect(onMQTTDisconnect);
    mqttClient.onMessage(onMQTTMessage);

    // Get connected
    connectToWiFi();
}

void loop()
{
    // The program loop does only 1 things...
    // Keeping the LED strip going
    ledstrip.service();
}
