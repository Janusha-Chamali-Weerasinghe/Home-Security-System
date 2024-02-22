// ======================================== Including the libraries.
#include <esp_now.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Arduino_JSON.h>
#include <BlynkSimpleEsp32.h>

#include "esp_camera.h"
#include "Arduino.h"
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "driver/rtc_io.h"
#include <SPIFFS.h>
#include <FS.h>
#include <Firebase_ESP_Client.h>
//Provide the token generation process info.
#include <addons/TokenHelper.h>
// ========================================

// Include the contents of the User Interface Web page, stored in the same folder as the .ino file
#include "PageIndex.h"


/* Fill-in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID           "TMPL6iTpWRvQs"
#define BLYNK_TEMPLATE_NAME         "ESP32"
#define BLYNK_AUTH_TOKEN            "7SZ1-JZNv8V844JFLcZgttLWpfTnyJbO"


// Defines the Digital Pin of the "On Board LED".
#define ON_Board_LED 2

//Define the Digital Pin of the Buzzer
#define Buzzer 12

// ======================================== Replace with your network credentials.
const char* wifi_network_ssid = "N6";  //--> Your wifi name
const char* wifi_network_password = "12345678901"; //--> Your wifi password
// ========================================

// ======================================== Access Point Declaration and Configuration.
const char* soft_ap_ssid = "ESP32_WS";  //--> access point name
const char* soft_ap_password = "helloesp32WS"; //--> access point password

IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
// ========================================

// ======================================== Variable for millis / timer.
unsigned long previousMillis = 0;
const long interval = 30000;
// ========================================

// ======================================== Define variables to store incoming readings.
String receive_ID_Sender = "";
String receive_Motion_Val;
String receive_Status_Read = "";
int receive_IsActive_PIR;
// ========================================

// ======================================== Structure example to receive data.
// Must match the sender structure
typedef struct struct_message_receive {
  String ID_Sender;
  String Motion;
  String Status_Read;
  int IsActive_PIR;
} struct_message_receive;

// Create a struct_message to receive data.
struct_message_receive receive_Data;
// ========================================

// Initialize JSONVar
JSONVar JSON_All_Data_Received;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create an Event Source on /events
AsyncEventSource events("/events");


//______________________________________________________________________________ Camera and firebase

// Insert Firebase project API Key
#define API_KEY "AIzaCSyDY-xWWjnsQXKpVkOubGdpA0qz0NRSWfGAKpsY"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "abc@gmail.com"
#define USER_PASSWORD "abc123"

// Insert Firebase storage bucket ID e.g bucket-name.appspot.com
#define STORAGE_BUCKET_ID "burglar-system-a149e.appspot.com"

// Photo File Name to save in SPIFFS
#define FILE_PHOTO "/data/photo.jpg"

// OV2640 camera module pins (CAMERA_MODEL_AI_THINKER)
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
#define Y3_GPIO_NUM        18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

boolean takeNewPhoto = true;

//Define Firebase Data objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig configF;

bool taskCompleted = false;

// Check if photo capture was successful
bool checkPhoto(fs::FS &fs) {
  File f_pic = fs.open(FILE_PHOTO);
  unsigned int pic_sz = f_pic.size();
  return (pic_sz > 100);
}

// Capture Photo and Save it to SPIFFS
void capturePhotoSaveSpiffs(void) {
  camera_fb_t *fb = NULL; // pointer
  bool ok = false; // Boolean indicating if the picture has been taken correctly
  do {
    // Take a photo with the camera
    Serial.println("Taking a photo...");

    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      return;
    }
    // Photo file name
    Serial.printf("Picture file name: %s\n", FILE_PHOTO);
    File file = SPIFFS.open(FILE_PHOTO, FILE_WRITE);
    // Insert the data in the photo file
    if (!file) {
      Serial.println("Failed to open file in writing mode");
    }
    else {
      file.write(fb->buf, fb->len); // payload (image), payload length
      Serial.print("The picture has been saved in ");
      Serial.print(FILE_PHOTO);
      Serial.print(" - Size: ");
      Serial.print(file.size());
      Serial.println(" bytes");
    }
    // Close the file
    file.close();
    esp_camera_fb_return(fb);

    // check if file has been correctly saved in SPIFFS
    ok = checkPhoto(SPIFFS);
  } while (!ok);
}


void initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    ESP.restart();
  }
  else {
    delay(500);
    Serial.println("SPIFFS mounted successfully");
  }
}

void initCamera() {
  // OV2640 camera module
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
  }
}

//______________________________________________________________________________


// ________________________________________________________________________________ Callback when data is received.
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  digitalWrite(ON_Board_LED, HIGH); //--> Turn on ON_Board_LED.

  // ---------------------------------------- Get the MAC ADDRESS of the sender / slave.
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  // ----------------------------------------

  memcpy(&receive_Data, incomingData, sizeof(receive_Data)); //--> Copy the information in the "incomingData" variable into the "receive_Data" structure variable.

  // ---------------------------------------- Prints the MAC ADDRESS of sender / slave and Bytes received.
  Serial.println();
  Serial.println("<<<<< Receive Data:");
  Serial.print("Packet received from: ");
  Serial.println(macStr);
  Serial.print("Bytes received: ");
  Serial.println(len);
  // ----------------------------------------

  // ---------------------------------------- Retrieve all data from "receive Data" structure variable.
  receive_ID_Sender = receive_Data.ID_Sender;
  receive_Motion_Val = receive_Data.Motion;
  receive_Status_Read = receive_Data.Status_Read;
  receive_IsActive_PIR = receive_Data.IsActive_PIR;
  // ----------------------------------------

  // ----------------------------------------

  // ---------------------------------------- Prints all data received from the sender.
  Serial.println("Receive Data: ");
  //  Serial.println(receive_ID_Sender);
  //  Serial.println(receive_Motion_Val);
  //  Serial.println(receive_Status_Read);
  //  Serial.println(receive_IsActive_PIR);

  Serial.printf("Siren : %s\n", receive_ID_Sender);
  Serial.printf("Motion : %s\n", receive_Motion_Val);
  Serial.printf("Status Read PIR Sensor : %s\n", receive_Status_Read);
  Serial.printf("PIR IS ACTIVE : %d\n", receive_IsActive_PIR);

  Serial.println("<<<<<");
  // ----------------------------------------

  digitalWrite(ON_Board_LED, LOW); //--> Turn off ON_Board_LED.
}
// ________________________________________________________________________________

// ________________________________________________________________________________ VOID SETUP()
void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  Serial.println();

  initSPIFFS();
  // Turn-off the 'brownout detector'
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  initCamera();

  pinMode(ON_Board_LED, OUTPUT); //--> On Board LED port Direction output
  digitalWrite(ON_Board_LED, LOW); //--> Turn off Led On Board

  pinMode(Buzzer, OUTPUT); //--> BUZZER port Direction output
  // ---------------------------------------- Set Wifi to AP+STA mode
  Serial.println();
  Serial.println("-------------");
  Serial.println("WIFI mode : AP + STA");
  WiFi.mode(WIFI_AP_STA);
  Serial.println("-------------");
  // ----------------------------------------

  // ---------------------------------------- Access Point Settings.
  Serial.println();
  Serial.println("-------------");
  Serial.println("WIFI AP");
  Serial.println("Setting up ESP32 to be an Access Point.");
  WiFi.softAP(soft_ap_ssid, soft_ap_password);
  delay(1000);
  Serial.println("Setting up ESP32 softAPConfig.");
  WiFi.softAPConfig(local_ip, gateway, subnet);
  Serial.println("-------------");
  // ----------------------------------------

  // ---------------------------------------- Connect to Wi-Fi (STA).
  Serial.println("------------");
  Serial.println("WIFI STA");
  Serial.print("Connecting to ");
  Serial.println(wifi_network_ssid);
  //Blynk.begin(BLYNK_AUTH_TOKEN, wifi_network_ssid, wifi_network_password);
  WiFi.begin(wifi_network_ssid, wifi_network_password);


  int connecting_process_timed_out = 20; //--> 20 = 20 seconds.
  connecting_process_timed_out = connecting_process_timed_out * 2;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(ON_Board_LED, HIGH);
    delay(250);
    digitalWrite(ON_Board_LED, LOW);
    delay(250);
    if (connecting_process_timed_out > 0) connecting_process_timed_out--;
    if (connecting_process_timed_out == 0) {
      delay(1000);
      ESP.restart();
    }
  }

  digitalWrite(ON_Board_LED, LOW);
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("------------");
  // ----------------------------------------

  // ---------------------------------------- Init ESP-NOW
  Serial.println();
  Serial.println("-------------");
  Serial.println("Start initializing ESP-NOW...");
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    Serial.println("Restart ESP32...");
    Serial.println("-------------");
    delay(1000);
    ESP.restart();
  }
  Serial.println("Initializing ESP-NOW was successful.");
  Serial.println("-------------");
  // ----------------------------------------

  // ---------------------------------------- Register for a callback function that will be called when data is received
  Serial.println();
  Serial.println("Register for a callback function that will be called when data is received");
  esp_now_register_recv_cb(OnDataRecv);
  // ----------------------------------------

  // ---------------------------------------- Handle Web Server
  Serial.println();
  Serial.println("Setting Up the Main Page on the Server.");
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", MAIN_page);
  });
  // ----------------------------------------

  // ---------------------------------------- Handle Web Server Events
  Serial.println();
  Serial.println("Setting up event sources on the Server.");
  events.onConnect([](AsyncEventSourceClient * client) {
    if (client->lastId()) {
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 10 second
    client->send("hello!", NULL, millis(), 10000);
  });
  // ----------------------------------------

  Serial.println();
  Serial.println("Adding event sources on the Server.");
  server.addHandler(&events);

  Serial.println();
  Serial.println("Starting the Server.");
  server.begin();

  Serial.println();
  Serial.println("------------");
  Serial.print("ESP32 IP address as soft AP : ");
  Serial.println(WiFi.softAPIP());
  Serial.print("ESP32 IP address on the WiFi network (STA) : ");
  Serial.println(WiFi.localIP());
  Serial.print("Wi-Fi Channel : ");
  Serial.println(WiFi.channel());
  Serial.println("------------");

  Serial.println();
  Serial.println("Visit the IP Address above in your browser to open the main page.");
  Serial.println("You can choose and use one of the IPs above.");
  Serial.println();

  //Firebase
  // Assign the api key
  configF.api_key = API_KEY;
  //Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  //Assign the callback function for the long running token generation task
  configF.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  Firebase.begin(&configF, &auth);
  Firebase.reconnectWiFi(true);
}
// ________________________________________________________________________________

//_________________________________________________________________________________ takePics

void takePics() {
  if (takeNewPhoto) {
    capturePhotoSaveSpiffs();
    delay(2000);
    if (Firebase.ready() && !taskCompleted) {
      taskCompleted = true;
      Serial.print("Uploading picture... ");

      //MIME type should be valid to avoid the download problem.
      //The file systems for flash and SD/SDMMC can be changed in FirebaseFS.h.
      if (Firebase.Storage.upload(&fbdo, STORAGE_BUCKET_ID /* Firebase Storage bucket id */, FILE_PHOTO /* path to local file */, mem_storage_type_flash /* memory storage type, mem_storage_type_flash and mem_storage_type_sd */, FILE_PHOTO /* path of remote file stored in the bucket */, "image/jpeg" /* mime type */)) {
        Serial.printf("\nDownload URL: %s\n", fbdo.downloadURL().c_str());

        JSON_All_Data_Received["idESP32Sender"] = receive_ID_Sender;
        JSON_All_Data_Received["motion"] = receive_Motion_Val;
        JSON_All_Data_Received["siren"] = "ACTIVATED";
        JSON_All_Data_Received["sensorstatus"] = receive_Status_Read;
        JSON_All_Data_Received["imgsrcc"] = fbdo.downloadURL().c_str();

        // Create a JSON String to hold all data received from the sender.
        String jsonString_Send_All_Data_received = JSON.stringify(JSON_All_Data_Received);
        // ----------------------------------------

        // ---------------------------------------- Sends all data received from the sender to the browser as an event ('allDataJSON').
        events.send(jsonString_Send_All_Data_received.c_str(), "allDataJSON", millis());

      }
      else {
        Serial.println(fbdo.errorReason());
      }
    }
    taskCompleted = false;
  }
  return;

}

//________________________________________________________________________________

// ________________________________________________________________________________ VOID LOOP()
void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(Buzzer, HIGH);
  if (receive_IsActive_PIR == 1) {
    takePics();
    receive_IsActive_PIR = 0;
  }
  //Blynk.run();
 
}
// ________________________________________________________________________________
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
