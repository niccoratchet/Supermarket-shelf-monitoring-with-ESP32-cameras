#ifndef MQTTCONNECTIONMANAGER_H
#define MQTTCONNECTIONMANAGER_H

#include "Config.h"
#include <string>
#include <Arduino.h>
#include <PubSubClient.h>

void sendMQTTMessage(PubSubClient* client, String topic, std::string results, int number_of_objects, bool isRetainedMessage, Config* config);
bool testMQTTConnection(PubSubClient* client, Config* config);

#endif
