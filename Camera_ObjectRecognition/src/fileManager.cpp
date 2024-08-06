#include <SPIFFS.h>
#include <FS.h>
#include "fileManager.h"

bool initializeFileSystem() {
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return false;
  }
  return true;
}

void verifyFilePresence(const char* fileName, const char* fileContent) {

  if (!SPIFFS.exists(fileName)) {                     // Controlla e carica il file HTML se non esiste
    File file = SPIFFS.open(fileName, "w");
    if (!file) {
      Serial.println(String("Error during the creation of: ") + fileName);
    }
    else {
      file.print(fileContent);
      file.close();
      Serial.println(String("Successfully created: ") + fileName);
    }
  }
  else {
    Serial.println(fileName + String(" already exists"));
  }

}

bool loadCredentials(Config* config) {

  File file = SPIFFS.open("/config.txt", "r");
  if(!file) {
    Serial.println("Error while opening the file 'config.txt'");
    return false;
  }
  
  size_t size = file.size();
  if (size == 0) {
    Serial.println("Empty file");
    return false;
  }

  std::unique_ptr<char[]> buf(new char[size + 1]);
  file.readBytes(buf.get(), size);
  buf[size] = '\0';

  Serial.println("File content:");
  Serial.println(buf.get());

  parseConfig(buf.get(), config);
  return true;

}

/**
    parseConfig() manages the extraction of data from the text file 'config.txt' and saving to the corresponding variables
*/

void parseConfig(const char* content, Config* config) {
    
  String line;
  String key;
  String value;

  for (size_t i = 0; i < strlen(content); i++) {
    if (content[i] == '\n' || content[i] == '\r') {
      if (line.length() > 0) {
        int delimiterIndex = line.indexOf('=');
        if (delimiterIndex > 0) {
          key = line.substring(0, delimiterIndex);
          value = line.substring(delimiterIndex + 1);

          if (key == "cameraID") config -> cameraID = value;
          else if (key == "ssid") config -> ssid = value;
          else if (key == "password") config -> password = value;
          else if (key == "mqtt_server") config -> mqtt_server = value;
          else if (key == "mqtt_username") config -> mqtt_username = value;
          else if (key == "mqtt_password") config -> mqtt_password = value;
        }
        line = "";
      }
    } else {
      line += content[i];
    }
  }

}

/*
    saveConfig() opens Wi-Fi and MQTT configuration file updating it with user's new information
*/

bool saveConfig(const char *filename, Config *config) {

  File file = SPIFFS.open(filename, "w");
  if (!file) {
    Serial.println("Error: it's impossible to open the 'config.txt' file");
    return false;
  }
  file.println("cameraID=1");
  file.println("ssid=" + config -> ssid);
  file.println("password=" + config -> password);
  file.println("mqtt_server=" + config -> mqtt_server);
  file.println("mqtt_username=" + config -> mqtt_username);
  file.println("mqtt_password=" + config -> mqtt_password);
  file.close();
  return true;

}