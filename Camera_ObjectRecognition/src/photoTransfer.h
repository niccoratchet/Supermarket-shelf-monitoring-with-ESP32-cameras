#ifndef PHOTO_TRANSFER_H
#define PHOTO_TRANSFER_H

#define SERVER_HOST "192.168.0.141"
#define SERVER_PORT 5000
#define SERVER_PATH "/upload"

#include <esp_camera.h>
#include <WiFi.h>

bool sendPhotoToWebServer(camera_fb_t* photoToTransfer);

#endif // PHOTO_TRANSFER_H