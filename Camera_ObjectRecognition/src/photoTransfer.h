#ifndef PHOTO_TRANSFER_H
#define PHOTO_TRANSFER_H

#define SERVER_PORT 5000
#define SERVER_PATH "/upload"

#include <esp_camera.h>
#include <WiFi.h>
#include <Config.h>

bool sendPhotoToWebServer(camera_fb_t* photoToTransfer, Config& config);

#endif // PHOTO_TRANSFER_H