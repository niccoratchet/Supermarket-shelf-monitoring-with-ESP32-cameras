#include "photoTransfer.h"

bool sendPhotoToWebServer(camera_fb_t* photoToTransfer, Config& config) {

    WiFiClient client;
    Serial.print("Connecting to: ");
    Serial.print(SERVER_HOST);
    Serial.print(":");
    Serial.println(SERVER_PORT);

    if (!client.connect(SERVER_HOST, SERVER_PORT)) {
        Serial.println("Connection failed");
        return false;
    }

    Serial.println("Connected to server");

    String boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";      // The boundary is used to separate the different parts of the request

    // Creating the body of the request multipart/form-data
    String bodyStart = "--" + boundary + "\r\n";
    bodyStart += "Content-Disposition: form-data; name=\"camera_id\"\r\n\r\n";
    bodyStart += String(config.cameraID) + "\r\n";
    bodyStart += "--" + boundary + "\r\n";
    bodyStart += "Content-Disposition: form-data; name=\"file\"; filename=\"photo.jpg\"\r\n";
    bodyStart += "Content-Type: image/jpeg\r\n\r\n";

    String bodyEnd = "\r\n--" + boundary + "--\r\n";

    // Calculate Content-Length accurately
    size_t content_length = bodyStart.length() + photoToTransfer->len + bodyEnd.length();

    // Creating the header of the HTTP request
    String header = "POST " + String(SERVER_PATH) + " HTTP/1.1\r\n";
    header += "Host: " + String(SERVER_HOST) + ":" + String(SERVER_PORT) + "\r\n";
    header += "Content-Type: multipart/form-data; boundary=" + boundary + "\r\n";
    header += "Content-Length: " + String(content_length) + "\r\n";
    header += "Connection: close\r\n\r\n";

    // Log the parts
    Serial.print("Header length: ");
    Serial.println(header.length());

    Serial.print("bodyStart length: ");
    Serial.println(bodyStart.length());

    Serial.print("Image length: ");
    Serial.println(photoToTransfer->len);

    Serial.print("bodyEnd length: ");
    Serial.println(bodyEnd.length());

    Serial.print("Total Content-Length: ");
    Serial.println(content_length);

    // Sending the header
    client.print(header);

    // Sending the body parts
    client.print(bodyStart);        // Sending the first part of the body
    client.write(photoToTransfer->buf, photoToTransfer->len);           // Sending the photo
    client.print(bodyEnd);          // Sending the last part of the body

    Serial.println("Photo sent");

    // Wait for the server's response
    while (client.connected()) {                           
        String line = client.readStringUntil('\n');
        if (line == "\r") {
            Serial.println("Headers received");
            break;
        }
    }

    // Read the response body
    String response = client.readString();
    Serial.println("Server response:");
    Serial.println(response);
    client.stop();

    if (response.equals("{\"message\": \"File uploaded successfully\"}") != -1) {             // If the response contains the message "File uploaded successfully" the photo has been sent correctly
        Serial.println("Photo sent correctly");
        return true;
    }
    else {
        Serial.println("Error while sending the photo");
        return false;
    }

}