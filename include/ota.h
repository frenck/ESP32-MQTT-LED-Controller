#ifndef OTA_H
#define OTA_H

#include <ArduinoOTA.h>

void setupOTA(const char *HOSTNAME, const uint16_t OTA_PORT,
              const char *OTA_PASSWORD);

#endif
