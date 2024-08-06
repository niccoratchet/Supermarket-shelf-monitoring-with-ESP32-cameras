#include "MQTTConnectionManager.h"

/**
 *  sendMQTTMessage() creates the connection to the MQTT server and sends a message on a certain topic
*/

void sendMQTTMessage(PubSubClient* client, String topic, std::string results, int number_of_objects, bool isRetainedMessage, Config* config) {

  while (!client -> connected()) {
    Serial.print("Attempting MQTT connection...");
    if(!isRetainedMessage) {
      if (client -> connect("ESP32Client", config -> mqtt_username.c_str(), config -> mqtt_password.c_str())) {
        Serial.println("Connected to the MQTT Broker for sending inference results");
        break;
      }
      else {
        Serial.print("failed, rc=");
        Serial.print(client -> state());
        Serial.println(" try again in 5 seconds");
        delay(5000);
      }
    }
    else {
      String disconnectionTopic = "disconnect/" + config -> cameraID;
      String disconnectionMessage = "Warning! Camera: " + config -> cameraID + " disconnected";
      if (client -> connect("ESP32Client", config -> mqtt_username.c_str(), config -> mqtt_password.c_str(), disconnectionTopic.c_str() , 0, true, disconnectionMessage.c_str())) {
        Serial.println("Connected to the MQTT Broker for sending setup info");
        break;
      }
      else {
        Serial.print("failed, rc=");
        Serial.print(client -> state());
        Serial.println(" try again in 5 seconds");
        delay(5000);
      }
    }
  }
  if(!isRetainedMessage) {
    if(number_of_objects >= 1) {
      client -> publish(topic.c_str(), results.c_str());
    }
    else {
        std::string no_objects = "No objects found";
        client -> publish(topic.c_str(), no_objects.c_str());
    }
  }
  else
    client -> publish(topic.c_str(), results.c_str());

}

/**
 *  testMQTTConnection() is used in the setup procedure in order to know if the MQTT Broker is reachable or not.
*/

bool testMQTTConnection(PubSubClient* client, Config* config) {

  unsigned long startAttemptTime = millis();
  while(!client -> connected() && millis() - startAttemptTime < 30000) {
    if (client -> connect("ESP32Client", config -> mqtt_username.c_str(), config -> mqtt_password.c_str())) {
      return true;
    }
  }
  return false;

}
