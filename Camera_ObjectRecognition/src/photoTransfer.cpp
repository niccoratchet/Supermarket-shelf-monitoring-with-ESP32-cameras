#include "photoTransfer.h"

#define SERVER_NAME "http://192.168.0.141:5000/upload"

bool sendPhotoToWebServer(camera_fb_t* photoToTransfer) {

    HTTPClient http;
    http.begin(SERVER_NAME);
    http.addHeader("Content-Type", "multipart/form-data");      // Set "multipart/form-data" as Content-Type in order to send the file

    // Creating the body of the request
    String boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";
    String bodyStart = "--" + boundary + "\r\n";
    bodyStart += "Content-Disposition: form-data; name=\"file\"; filename=\"photo.jpg\"\r\n";
    bodyStart += "Content-Type: image/jpeg\r\n\r\n";
    String bodyEnd = "\r\n--" + boundary + "--\r\n";
    
    http.addHeader("Content-Type", "multipart/form-data; boundary=" + boundary);        // Send the start of the form data request
    
    int httpResponseCode = http.POST(bodyStart + (char*)photoToTransfer->buf + bodyEnd);        // Send the photo to the server

    if (httpResponseCode == 200) {
        Serial.print("HTTP Response code: ");
        printHttpResponse(httpResponseCode, http);
        return true;
    }
    else {
        Serial.print("Error code: ");
        printHttpResponse(httpResponseCode, http);
        return false;
    }

}

void printHttpResponse(int httpResponseCode, HTTPClient &http) {

    Serial.println(httpResponseCode);
    String response = http.getString();
    Serial.println(response);
    http.end();

}