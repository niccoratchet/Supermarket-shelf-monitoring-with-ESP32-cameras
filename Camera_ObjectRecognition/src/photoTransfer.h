#include <Arduino.h>
#include <esp_camera.h>
#include <WiFi.h>
#include <HTTPClient.h>

bool sendPhotoToWebServer(camera_fb_t* photoToTransfer);