#ifndef CONFIG_H
#define CONFIG_H

#include <WString.h>

struct Config {               // Data structure used for saving Wi-Fi and MQTT connection parameters.
  /*
    Every camera has his unique ID. This is used by the backend server to identify which camera is sending data.
    This value is set when the camera is used for the first time.
  */ 
  String cameraID;

  // Wi-Fi credantials
  String ssid;
  String password;

  // MQTT Broker connection parameters. "mqtt_server" is mandatory, the need of the others tow depends on the MQTT server we want to connect to.
  String mqtt_server;
  String mqtt_username;
  String mqtt_password;

};

#endif