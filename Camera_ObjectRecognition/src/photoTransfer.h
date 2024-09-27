#ifndef PHOTO_TRANSFER_H
#define PHOTO_TRANSFER_H

#include <Arduino.h>
#include <esp_camera.h>
#include <WiFi.h>
#include <HTTPClient.h>

bool sendPhotoToWebServer(camera_fb_t* photoToTransfer);
void printHttpResponse(int httpResponseCode, HTTPClient &http);

#endif // PHOTO_TRANSFER_H