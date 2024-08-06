#ifndef fileManager_H
#define fileManger_H
#include "Config.h"

bool initializeFileSystem();
void verifyFilePresence(const char* fileName, const char* fileContent);
bool loadCredentials(Config* config);
void parseConfig(const char* content, Config* config);
bool saveConfig(const char *filename, Config *config);

#endif

