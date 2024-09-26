#include "photoTransfer.h"

#define SERVER_NAME "http://127.0.0.1:5000/upload"

bool sendPhotoToWebServer(camera_fb_t* photoToTransfer) {

    HTTPClient http;                                                // Setting up the HTTP client in order to send the photo to the web server
    http.begin(SERVER_NAME);
    http.addHeader("Content-Type", "image/jpeg");

    int httpResponseCode = http.POST((uint8_t*)photoToTransfer->buf, photoToTransfer->len);         // Sending the photo to the web server

    if (httpResponseCode > 0) {                                         // Checking if the photo was sent successfully
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        http.end();
        return true;
    }
    else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
        http.end();
        return false;
    }

}