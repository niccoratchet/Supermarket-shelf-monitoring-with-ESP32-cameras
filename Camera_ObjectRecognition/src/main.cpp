#include <Arduino.h>
#include <WiFi.h>
#include "esp_camera.h"
#include <PubSubClient.h>
#include <FS.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include "edge-impulse-sdk/classifier/ei_run_classifier.h"
#include "edge-impulse-sdk/dsp/image/processing.hpp"
#include "fileManager.h"
#include "Config.h"
#include "MQTTConnectionManager.h"
#include "photoTransfer.h"

/**
    NOTE: The library used for importing the ML model is .ZIP file: "timer_camera_object_recognition-V23.zip" where "timer_camera_object_recognition" is the name of the project in Edge Impulse, 
    and "V23" is the version of the library (it can be different if you have a different project name and different version).
    The fact that the model is separated from this program permits us to change it whenever we want.
    This can happen when we want to get a better trained model: once we obtained it, we can use it in this program importing the exact same library.
    In order to change the library version, you have to first delete the previous one. You can go to your project folder and delete folders: "edge-impulse-sdk", "model-parameters", "tflite-model".
    Then you can import the new library version's folders in the same location inside the project folder.
*/

//#define CAMERA_MODEL_ESP_EYE // Has PSRAM
//#define CAMERA_MODEL_AI_THINKER // Has PSRAM
//#define CAMERA_MODEL_M5STACK_ESP32CAM
#define CAMERA_MODEL_M5STACK_PSRAM

#if defined(CAMERA_MODEL_ESP_EYE)
#define PWDN_GPIO_NUM    -1
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    4
#define SIOD_GPIO_NUM    18
#define SIOC_GPIO_NUM    23

#define Y9_GPIO_NUM      36
#define Y8_GPIO_NUM      37
#define Y7_GPIO_NUM      38
#define Y6_GPIO_NUM      39
#define Y5_GPIO_NUM      35
#define Y4_GPIO_NUM      14
#define Y3_GPIO_NUM      13
#define Y2_GPIO_NUM      34
#define VSYNC_GPIO_NUM   5
#define HREF_GPIO_NUM    27
#define PCLK_GPIO_NUM    25

#elif defined(CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22


#elif defined(CAMERA_MODEL_M5STACK_ESP32CAM)
#define PWDN_GPIO_NUM  -1
#define RESET_GPIO_NUM 15
#define XCLK_GPIO_NUM  27
#define SIOD_GPIO_NUM  25
#define SIOC_GPIO_NUM  23

#define Y9_GPIO_NUM    19
#define Y8_GPIO_NUM    36
#define Y7_GPIO_NUM    18
#define Y6_GPIO_NUM    39
#define Y5_GPIO_NUM    5
#define Y4_GPIO_NUM    34
#define Y3_GPIO_NUM    35
#define Y2_GPIO_NUM    17
#define VSYNC_GPIO_NUM 22
#define HREF_GPIO_NUM  26
#define PCLK_GPIO_NUM  21

#elif defined(CAMERA_MODEL_M5STACK_PSRAM)
#define PWDN_GPIO_NUM  -1
#define RESET_GPIO_NUM 15
#define XCLK_GPIO_NUM  27
#define SIOD_GPIO_NUM  25
#define SIOC_GPIO_NUM  23

#define Y9_GPIO_NUM    19
#define Y8_GPIO_NUM    36
#define Y7_GPIO_NUM    18
#define Y6_GPIO_NUM    39
#define Y5_GPIO_NUM    5
#define Y4_GPIO_NUM    34
#define Y3_GPIO_NUM    35
#define Y2_GPIO_NUM    32
#define VSYNC_GPIO_NUM 22
#define HREF_GPIO_NUM  26
#define PCLK_GPIO_NUM  21

#else
#error "Camera model not selected"
#endif

/**
 *   NOTE: The 2 following parameters have to match the photo resolution present in the following object "camera_config".
           For example, if in camera_config we have ".frame_size = FRAMESIZE_QVGA", these two parameters has to be 320 & 240 (QVGA = 320x240).
           If not, the program will crash when it will be asked to do the object recognition.
*/

#define EI_CAMERA_RAW_FRAME_BUFFER_COLS           320
#define EI_CAMERA_RAW_FRAME_BUFFER_ROWS           240
#define EI_CAMERA_FRAME_BYTE_SIZE                 3

static bool debug_nn = false;                   // Set this to true to see e.g. features generated from the raw signal
static bool is_initialised = false;
uint8_t *snapshot_buf;                          // Points to the output of the capture
float minConfidence = 0.85;
camera_fb_t *photoToTransfer = NULL;            // It is used to store the photo taken by the camera and then transfer it to the back-end

static camera_config_t camera_config = {        // Values definition about the struct "camera_config": it is used to configure parameters like PINs on the board, photo's quality and resolution ecc..

    .pin_pwdn = PWDN_GPIO_NUM,
    .pin_reset = RESET_GPIO_NUM,
    .pin_xclk = XCLK_GPIO_NUM,
    .pin_sscb_sda = SIOD_GPIO_NUM,
    .pin_sscb_scl = SIOC_GPIO_NUM,

    .pin_d7 = Y9_GPIO_NUM,
    .pin_d6 = Y8_GPIO_NUM,
    .pin_d5 = Y7_GPIO_NUM,
    .pin_d4 = Y6_GPIO_NUM,
    .pin_d3 = Y5_GPIO_NUM,
    .pin_d2 = Y4_GPIO_NUM,
    .pin_d1 = Y3_GPIO_NUM,
    .pin_d0 = Y2_GPIO_NUM,
    .pin_vsync = VSYNC_GPIO_NUM,
    .pin_href = HREF_GPIO_NUM,
    .pin_pclk = PCLK_GPIO_NUM,

    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG,           // Formats supported: YUV422,GRAYSCALE,RGB565,JPEG. Using JPEG is highly recommended
    .frame_size = FRAMESIZE_QVGA,             // Supported resolutions: QQVGA-UXGA. Do not use sizes above QVGA when not JPEG

    .jpeg_quality = 12,                       // It goes from 0 to 63. Lower values mean higher photo quality.

    .fb_count = 1,                            // If more than one, i2s runs in continuous mode. Use only with JPEG
    .fb_location = CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,

};

// HTML page used by the user for configuring Wi-Fi e and MQTT connection parameters
const char* config_html = R"rawliteral(         
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 Wi-Fi an MQTT Configuration</title>
  <script>
    function validateForm() {
      var wifi_ssid = document.getElementById('wifi_ssid').value;
      var mqtt_server = document.getElementById('ftp_server').value;

      if (!wifi_ssid || !mqtt_server) {
        alert('Please, insert at least the following parameters: Wi-Fi SSID, MQTT address');
        return false;
      }
      return true;
    }
  </script>
</head>
<body>
  <h1>Wi-Fi and MQTT configuration for ESP32 cameras</h1>
  <form action="/save" method="POST" onsubmit="return validateForm()">
    <fieldset>
      <legend>Wi-Fi credentials</legend>
      <label for="wifi_ssid">SSID:</label><br>
      <input type="text" id="wifi_ssid" name="wifi_ssid"><br />
      <label for="wifi_password">Password:</label><br />
      <input type="password" id="wifi_password" name="wifi_password"><br />
    </fieldset> <br />
    <fieldset>
      <legend>MQTT Broker connection parameters</legend>
      <label for="mqtt_server">Address</label><br />
      <input type="text" id="mqtt_server" name="mqtt_server"><br />
      <label for="mqtt_user">User:</label><br />
      <input type="text" id="mqtt_user" name="mqtt_user"><br />
      <label for="mqtt_password">Password:</label><br />
      <input type="password" id="mqtt_password" name="mqtt_password"><br /><br />
    </fieldset>
    <br /><input type="submit" value="Save configuration">
  </form>
</body>
</html>
)rawliteral";

//  HTML page containing a message to the user about the successful camera configuration
const char* confirmationPage_html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Configuration completed!</title>
</head>
<body>
  <h3>ESP32 Camera is ready!</h3>
  <p>The camera is now connected to the MQTT Broker and it will starts to send image recognition results at topic 'inference/#'.<br />You can close this page.</p>
</body>
</html>
)rawliteral";

// Global variables
Config config;
WiFiClient espClient;
PubSubClient client(espClient);
String MQTTTopic;
WebServer server(80);
bool serverActive = true;
unsigned long startWaitingTime;
const unsigned long waitingDuration = 100000;         // It indicates the time the camera will wait until it start the configuration process with the parameters saved in the 'config.txt' file
volatile bool isIDSet = false;                              // It indicates if the camera has already an ID set by the back-end

// Functions definition
bool setup_wifi();
void configWebServer();
void openCompleteConfigurationPage();
void scanForObjects();
bool ei_camera_init(void);
void ei_camera_deinit(void);
bool ei_camera_capture(uint32_t img_width, uint32_t img_height, uint8_t *out_buf);
static int ei_camera_get_data(size_t offset, size_t length, float *out_ptr);
bool sendSetUpInformation();

/**
    setup_wifi() permits to connect the camera at the Wi-FI network
*/

bool setup_wifi() {
 
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(config.ssid.c_str());
  WiFi.begin(config.ssid.c_str(), config.password.c_str());

  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 30000) {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    return true;
  }
  else
    return false;
 
}

/**
    sendSetUpInformation() sends a message to the back-end server requesting a new ID. The camera will wait for the ID to be set by the back-end listening to the topic "newID/#".
    After that the camera will unsubscribe from the topic and the configuration process will be completed.
*/

bool sendSetUpInformation() {

  String inferenceCategories = "";
  int dimension = sizeof(ei_classifier_inferencing_categories) / 4;
  for (int i = 0; i < dimension-1; i++)
  {
      inferenceCategories = inferenceCategories + ei_classifier_inferencing_categories[i]+ " ";
  }
  inferenceCategories = inferenceCategories + ei_classifier_inferencing_categories[dimension-1];
  Serial.println("Sending configuration info");
  String connectedCameraTopic = "cameraConnected/N";                          // The camera sends a message to the MQTT Broker in order to inform that needs a new ID

  Serial.println("Subscribing to topic newID/#");                             // The camera subscribes to the topic "newID/#" in order to receive the camera's ID
  if (client.subscribe("newID/#")) {
      Serial.println("Subscription successful");
  } else {
      Serial.println("Subscription failed");
      return false;
  }

  sendMQTTMessage(&client, connectedCameraTopic, inferenceCategories.c_str(), 0, true, &config);    // The camera sends the message to the MQTT Broker requesting a new ID

  unsigned long startAttemptTime = millis();
  while (!isIDSet && millis() - startAttemptTime < waitingDuration) {           // The camera waits for the ID to be set by the back-end. If the time is over, the camera will stop the configuration process
    client.loop();
    delay(100);
  }
  if(isIDSet) {
    Serial.println("\nCamera ID set to: " + config.cameraID);
    client.unsubscribe("newID/#");                                              // Once the camera has the ID, it unsubscribes from the topic "newID/#"
    return true;
  }
  else {
    Serial.println("\nCamera ID not set. Please, check the back-end configuration and try again reconnecting the camera");
    return false;
  }

}

/**
 *  callback() is the function called when the camera receives the message from the back-end that contains the camera's ID.
 */

void callback(char* topic, byte* payload, unsigned int length) {

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  String topicString = String(topic);
  if(topicString.startsWith("newID/")) {
    isIDSet = true;
    config.cameraID = message;
    Serial.println("Camera ID set to: " + config.cameraID);
  }
  else {
    Serial.println("Unknown topic: " + topicString);
  }

}

/**
    configWebServer() as the name say, configures the web server's response to all the HTML requests by the user (sending the 'config.html' and saving form's contents)
*/

void configWebServer() {
  
  WiFi.softAP("ESP32 Camera Configuration");              // The camera creates an Access Point in order to let the user connect to it and configure the camera
  IPAddress IP = WiFi.softAPIP();
  Serial.print("Access Point started. Connect to 'ESP32 Camera Configuration' e access via browser the address http://");
  Serial.println(IP);

  /*
      "server.on" configures the HTTP server's response to a client's request via browser when trying to access at the previous defined IP address and so when GET method is executed.
      In this case, the server will send the HTML page 'config.html' to the client.
  */

  server.on("/", HTTP_GET, []() {                             
    File file = SPIFFS.open("/config.html", "r");
    if (!file) {
      server.send(500, "text/plain", "Error loading the configuration page");
      return;
    }
    String html = file.readString();
    server.send(200, "text/html", html);
    file.close();
  });

  /*
      This "server.on" defines the response of the HTTP POST method: it saves the form's data on the config.txt file.
  */

  server.on("/save", HTTP_POST, []() {
    config.ssid = server.arg("wifi_ssid");
    config.password = server.arg("wifi_password");
    config.mqtt_server = server.arg("mqtt_server");
    config.mqtt_username = server.arg("mqtt_user");
    config.mqtt_password = server.arg("mqtt_password");
    config.cameraID = "N";                                       // "N" because the cameraID is not known yet, it will sent by the back-end

    if(setup_wifi()) {
        if(testMQTTConnection(&client, &config)) {
          Serial.println("MQTT Broker reachable");
          if(sendSetUpInformation()) {
               if(saveConfig("/config.txt", &config)) {
                  openCompleteConfigurationPage();
                  WiFi.softAPdisconnect(true);
                  Serial.println("Access Point stopped");
                  MQTTTopic = "inference/" + config.cameraID;       // The cameraID is now known, so the MQTT topic can be set up
                  while (true)
                    scanForObjects();
               }
               else {
                  server.send(500, "text/plain", "Error during credentials saving");
               }
          }
          else {
            server.send(500, "text/plain", "Error during MQTT connection. Credentials could be uncorrected or the specified server is not reachable. Go back and retry. \n If the problem persists, try to unplug and then replug the camera");
          }
        }
        else
          server.send(500, "text/plain", "Error during MQTT connection. Credentials could be uncorrected or the specified server is not reachable. Go back and retry. \n If the problem persists, try to unplug and then replug the camera");
    }
    else {
        server.send(500, "text/plain", "Error during Wi-FI connection. Check if Wi-Fi SSID and password are correct, or verify if there's enough signal. Go back to previous page to retry.");
    }
  });
  server.begin();
  Serial.println("Web Server started");
  
}


/**
    openCompleteConfigurationPage() opens the final page that tells the user the camera is now sending inference's results to the MQTT Broker.
*/

void openCompleteConfigurationPage() {

  File file = SPIFFS.open("/confirmationPage.html", "r");
  if (!file) {
    server.send(500, "text/plain", "Error loading the final page. It should display only a confirmation page, so the rest of the program should be fine. You can now see MQTT messages send to the MQTT Broker");
    return;
  }
  String html = file.readString();
  server.send(200, "text/html", html);
  file.close();

  unsigned long startAttemptTime = millis();
  while(millis() - startAttemptTime < 5000) {
    delay(100);
  }

}

/**
    scanForObjects() will do the inference in the ML Model created in Edge Impulse. It will display the results in the serial connection and send them to the MQTT Broker
*/

void scanForObjects() {
  
    if (ei_sleep(5) != EI_IMPULSE_OK) {
        return;
    }
    snapshot_buf = (uint8_t*)malloc(EI_CAMERA_RAW_FRAME_BUFFER_COLS * EI_CAMERA_RAW_FRAME_BUFFER_ROWS * EI_CAMERA_FRAME_BYTE_SIZE);       // It creates a buffer where all the photo data is stored
    
    if(snapshot_buf == nullptr) {                                         // It checks if photo's buffer allocation was successful
        ei_printf("ERR: Failed to allocate snapshot buffer!\n");
        return;
    }
    ei::signal_t signal;
    signal.total_length = EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT;
    signal.get_data = &ei_camera_get_data;

    if (ei_camera_capture((size_t)EI_CLASSIFIER_INPUT_WIDTH, (size_t)EI_CLASSIFIER_INPUT_HEIGHT, snapshot_buf) == false) {
        ei_printf("Failed to capture image\r\n");
        free(snapshot_buf);
        return;
    }
    
    ei_impulse_result_t result = { 0 };
    EI_IMPULSE_ERROR err = run_classifier(&signal, &result, debug_nn);    // It runs the image classification process using the photo contained in the signal object

    if (err != EI_IMPULSE_OK) {
        ei_printf("ERR: Failed to run classifier (%d)\n", err);
        return;
    }
    /*
        Now scan's results are printed. There are 3 main parameters:
        DSP Timing: time (in ms) needed in order to capture and elaborate the photo signal. The elaboration includes filtering operations, data transformation and extracting relevant characteristics from the starting signal.
        Classification: time (in ms) needed in order to complete the model's inferation (so displaying what the objects seen by the camera and confidence percentage)
        Anomaly: time (in ms) needed to scan the image if there was an anomaly. This parameter is considered when the model is trained for detecting data's strange behaviour (not our case)
    */

    std::string outputString;
    int number_of_objects = 0;                                                     // Output string that contains all the elements recognized

    ei_printf("Predictions (DSP: %d ms., Classification: %d ms., Anomaly: %d ms.): \n",
                result.timing.dsp, result.timing.classification, result.timing.anomaly);
    ei_printf("Object detection bounding boxes:\r\n");
    for (uint32_t i = 0; i < result.bounding_boxes_count; i++) {
        ei_impulse_result_bounding_box_t bb = result.bounding_boxes[i];           // The "bb" object represents a subject detected by the camera in the photo. All detected subjects are stored in the vector "result.bounding_boxes"
        if (bb.value == 0) {                                                      // Its skips the iteration if a certain bb doesn't contain any subjects
            continue;
        }

        /*
            In case that bb contains a subject, it will be printed the subject's label, the confidence (in % value), his position in the photo (x,y), height length of bounding box drawn around the subject.
            The object would be considered as "detected" only if the confidence level would be higher than "minConfidence" (choosen by the user, it's recommended to use values from 0,85 and higher)
        */

        if(bb.value >= minConfidence) {
            ei_printf("  %s (%f) [ x: %u, y: %u, width: %u, height: %u ]\r\n",
                bb.label,
                bb.value,
                bb.x,
                bb.y,
                bb.width,
                bb.height);
            char buffer[256];             // Temporary buffer in order to store results correctly
            std::sprintf(buffer, "  %s (%f) [ x: %u, y: %u, width: %u, height: %u ]\r\n",
                     bb.label, bb.value, bb.x, bb.y, bb.width, bb.height);
            outputString += buffer;
            number_of_objects++;
        }
    }

    if (!client.connected()) {        // If the MQTT connection is set up, the camera sends the model's inferation results to the MQTT broker
      sendMQTTMessage(&client, MQTTTopic, outputString, number_of_objects, false, &config);
      if (photoToTransfer != NULL) {
        if(sendPhotoToWebServer(photoToTransfer)) {
          Serial.println("Photo sent to the web server");
        }
        else
          Serial.println("Something went wrong during the image transportation to the Web Server");
        // delete photoToTransfer;
        photoToTransfer = NULL;
      }
      
    }
    client.loop();
    free(snapshot_buf);
    delay(20000);

}


/**
    ei_camera_init() initializes the camera. Some of the instructions are specific for a certain camera model. It returns "true" if all setup is OK.
*/

bool ei_camera_init(void) {

    if (is_initialised) return true;

#if defined(CAMERA_MODEL_ESP_EYE)                   // Instructions like this are pre-processor directives for including or not some code specific for certain camera models 
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif
    esp_err_t err = esp_camera_init(&camera_config);    // Camera's initialization
    if (err != ESP_OK) {
      Serial.printf("Camera init failed with error 0x%x\n", err);
      return false;
    }
    sensor_t * s = esp_camera_sensor_get();
    if (s->id.PID == OV3660_PID) {                // This 4 instructions are used to set image correctly. Initial sensors are flipped vertically and colors are a bit saturated.
      s->set_vflip(s, 1);                         // Flip the camera framing back
      s->set_brightness(s, 1);                    // Up the brightness just a bit
      s->set_saturation(s, 0);                    // Lower the saturation
    }

#if defined(CAMERA_MODEL_M5STACK_WIDE)
    s->set_vflip(s, 1);
    s->set_hmirror(s, 1);
#elif defined(CAMERA_MODEL_ESP_EYE)
    s->set_vflip(s, 1);
    s->set_hmirror(s, 1);
    s->set_awb_gain(s, 1);
#endif

    is_initialised = true;
    return true;

}

/**
      ei_camera_deinit() deinitialize the camera when the image analysis is completed.
 */
void ei_camera_deinit(void) {

    esp_err_t err = esp_camera_deinit();
    if (err != ESP_OK)
    {
        ei_printf("Camera deinit failed\n");
        return;
    }
    is_initialised = false;
    return;

}


/**
      ei_camera_capture() captures the photo at the initial resolution set in the "camera_config" struct.
      If the model is trained in a resolution that is lower than the photo one, the function will do a resize to adapt the native image.
      This is necessary in order to do a correct inference with the trained model.
 */

bool ei_camera_capture(uint32_t img_width, uint32_t img_height, uint8_t *out_buf) {

  bool do_resize = false;
  if (!is_initialised) {
      ei_printf("ERR: Camera is not initialized\r\n");
      return false;
  }
  camera_fb_t *fb = esp_camera_fb_get();          // The camera will take a photo. This will be stored and pointed by "*fb".
  if (!fb) {
      ei_printf("Camera capture failed\n");
      return false;
  }
  photoToTransfer = fb;                           // The photo taken by the camera is stored in the "photoToTransfer" object
  bool converted = fmt2rgb888(fb->buf, fb->len, PIXFORMAT_JPEG, snapshot_buf);
  esp_camera_fb_return(fb);
  if(!converted){
      ei_printf("Conversion failed\n");
      delete photoToTransfer;                           // In case something goes wrong, the photo taken by the camera is deleted in order to not have memory leaks
      return false;
  }
  if ((img_width != EI_CAMERA_RAW_FRAME_BUFFER_COLS)
   || (img_height != EI_CAMERA_RAW_FRAME_BUFFER_ROWS)) {  
    do_resize = true;
  }
  if (do_resize) {
        ei::image::processing::crop_and_interpolate_rgb888(
        out_buf,
        EI_CAMERA_RAW_FRAME_BUFFER_COLS,
        EI_CAMERA_RAW_FRAME_BUFFER_ROWS,
        out_buf,
        img_width,
        img_height);
    }
  return true;
  
}

/**
    Once the photo has been captured, ei_camera_get_data() will elaborate the image in order to make it readable for the trained model.
*/

static int ei_camera_get_data(size_t offset, size_t length, float *out_ptr) {

    size_t pixel_ix = offset * 3;
    size_t pixels_left = length;
    size_t out_ptr_ix = 0;

    while (pixels_left != 0) {
        // The following instruction swap BGR to RGB here. This is needed due to a different color management for ESP32 camera models (see https://github.com/espressif/esp32-camera/issues/379)
        out_ptr[out_ptr_ix] = (snapshot_buf[pixel_ix + 2] << 16) + (snapshot_buf[pixel_ix + 1] << 8) + snapshot_buf[pixel_ix];
        out_ptr_ix++;   // go to the next pixel
        pixel_ix+=3;
        pixels_left--;
    }
    return 0;

}

void setup() {
 
  Serial.begin(115200);
  Serial.println("Starting setup...");
  if(!initializeFileSystem()) {
    Serial.println("Error while initializing the file system");
    return;
  }
  verifyFilePresence("/config.html", config_html);
  verifyFilePresence("/confirmationPage.html", confirmationPage_html);
  client.setServer(config.mqtt_server.c_str(), 1883);
  if (ei_camera_init() == false) {
      ei_printf("Failed to initialize Camera!\r\n");
  }
  else {
      ei_printf("Camera initialized\r\n");
  }
  client.setCallback(callback);                 // It sets the callback function that will be called when the camera receives a message from the back-end
  configWebServer();
  startWaitingTime = millis();
  Serial.println("Waiting for user's configuration...");

}

void loop() {

  unsigned long currentTime = millis();
  if (serverActive) {
    server.handleClient();
    if(currentTime - startWaitingTime >= waitingDuration) {       // Starting waiting for 5 minutes before using already existing parameters in the 'config.txt' file
      serverActive = false;
      Serial.println("Timer expired, stopping server...");
      server.stop();
      if(!loadCredentials(&config)) {
        Serial.println("Error while extracting configuration data from 'config.txt'.");
        return;
      }
      if (setup_wifi()) {
        if(testMQTTConnection(&client, &config)) {
          Serial.println("MQTT Broker reachable");
          WiFi.softAPdisconnect(true);
          Serial.println("Access Point stopped");
          MQTTTopic = "inference/" + config.cameraID;     
        }
      }
    }
  }
  else {
    scanForObjects();
  }
  
}

#if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_CAMERA
#error "Invalid model for current sensor"
#endif