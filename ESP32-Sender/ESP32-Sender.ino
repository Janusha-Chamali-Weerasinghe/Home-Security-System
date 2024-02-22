/*
   ESP32-DevKit-Sender-1
*/

// ======================================== Including the libraries.
#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_sleep.h>
#include <WiFi.h>
// ========================================

// Defines the Digital Pin of the "On Board LED".
#define ON_Board_LED 2

//PIR Sensor
const int PIN_TO_PIR_SENSOR = 12;

//LED
const int PIN_TO_RED_LED = 13;
const int PIN_TO_GREEN_LED = 14;

// Variables for PIR data
int int_value;
int pinStateCurrent   = LOW;
int pinStatePrevious  = LOW;

// ======================================== REPLACE WITH THE MAC ADDRESS OF YOUR MASTER / RECEIVER / ESP32 RECEIVER.
uint8_t broadcastAddress[] = {0xA8, 0x42, 0xE3, 0x4A, 0x1F, 0x6C};
//0xA8, 0x42, 0xE3, 0x4A, 0x1F, 0x6C
// ========================================

// Insert your SSID (The SSID of the "ESP32 Receiver" access point.).
constexpr char WIFI_SSID[] = "ESP32_WS";

// ======================================== Variable for millis / timer.
unsigned long previousMillisSend = 0;
const long intervalSend = 5000;

unsigned long previousMillisScan = 0;
const long intervalScan = 60000;
// ========================================

// ======================================== Variables to accommodate the data to be sent.
String send_ID_Board = "#1";
String send_Motion = "";
//String send_Siren = "";
String send_Status = "";
int send_IsActive_PIR;
// ========================================

// ======================================== Structure example to send data
// Must match the receiver structure
typedef struct struct_message_send {
  String ID_Board;
  String Motion;
  //String Siren;
  String Status_Read;
  int IsActive_PIR;
} struct_message_send;

struct_message_send send_Data; //--> Create a struct_message to send data.
// ========================================

// ________________________________________________________________________________ Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  digitalWrite(ON_Board_LED, HIGH); //--> Turn on ON_Board_LED when it starts sending data.
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  Serial.println(">>>>>");
  digitalWrite(ON_Board_LED, LOW); //--> Turn off ON_Board_LED when finished sending data.
}

int32_t getWiFiChannel(const char *ssid) {
  if (int32_t n = WiFi.scanNetworks()) {
    for (uint8_t i = 0; i < n; i++) {
      if (!strcmp(ssid, WiFi.SSID(i).c_str())) {
        return WiFi.channel(i);
      }
    }
  }
  return 0;
}
// ________________________________________________________________________________

// ________________________________________________________________________________ Subroutine to scan the access point of "ESP32 Receiver" to get the Wi-Fi channel used by "ESP32 Receiver".
void scan_and_set_Wifi_Channel() {
  // "ESP32 Sender" and "ESP32 Receiver" must use the same Wi-Fi channel.

  Serial.println();
  Serial.println("-------------");
  Serial.print("Scanning SSID : ");
  Serial.print(WIFI_SSID);
  Serial.println(" ...");

  // Get the Wi-Fi channel "ESP32 Receiver".
  int32_t channel = getWiFiChannel(WIFI_SSID);

  // Get the Wi-Fi channel on this device (ESP32 Sender).
  int cur_WIFIchannel = WiFi.channel();

  if (channel == 0) {
    Serial.print("SSID : ");
    Serial.print(WIFI_SSID);
    Serial.println(" not found !");
    Serial.println();
  } else {
    Serial.print("SSID : ");
    Serial.print(WIFI_SSID);
    Serial.print(" found. (Channel : ");
    Serial.print(channel);
    Serial.println(")");

    // If the "ESP32 Sender" Wi-Fi channel is different from the "ESP32 Receiver" Wi-Fi channel,
    // then the "ESP32 Sender" Wi-Fi channel is set to use the same channel as the "ESP32 Receiver" Wi-Fi channel.
    if (cur_WIFIchannel != channel) {
      Serial.println("Set Wi-Fi channel...");
      Serial.println();
      esp_wifi_set_promiscuous(true);
      esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
      esp_wifi_set_promiscuous(false);
    }
  }

  Serial.print("Wi-Fi channel : ");
  Serial.println(WiFi.channel());
  Serial.println("-------------");
}
// ________________________________________________________________________________

// ________________________________________________________________________________ Subroutine to read and get data from the PIR sensor.
void read_and_get_PIR_sensor_data() {


  //Check PIR State
  pinStatePrevious = pinStateCurrent;
  pinStateCurrent = digitalRead(PIN_TO_PIR_SENSOR);

  if (pinStatePrevious == LOW && pinStateCurrent == HIGH) {
    Serial.println("\nMotion detected! Waking up...");
    //Serial.println("\nMotion detected!");

    Serial.println();
    Serial.println("-------------");
    Serial.print("ESP32 ");
    Serial.println(send_ID_Board);

    send_Motion = "DETECTED";
    //send_Siren = "ACTIVATED";
    send_IsActive_PIR = 1;

    // Check if any reads failed.
    if (send_Motion == "") {
      Serial.println("Failed to read ");
      send_Motion = "Nan";
      //send_Siren = "Nan";
      send_Status = "FAILED";
    } else {
      send_Status = "SUCCEED";
    }

    // ::::::::::::::::: Set data to send
    send_Data.ID_Board = send_ID_Board;
    send_Data.Motion = send_Motion;
    //send_Data.Siren = send_Siren;
    send_Data.Status_Read = send_Status;
    send_Data.IsActive_PIR = send_IsActive_PIR;
    // :::::::::::::::::

    Serial.println();
    Serial.print(">>>>> ");
    Serial.println("Send data");

    delay(2000);
    digitalWrite(PIN_TO_GREEN_LED, LOW);
    digitalWrite(PIN_TO_RED_LED, HIGH);

    Serial.printf("Motion : %s\n", send_Motion);
    //Serial.printf("Siren : %s\n", send_Siren);
    Serial.printf("Status Read PIR Sensor : %s\n", send_Status);
    Serial.printf("PIR IS ACTIVE : %d\n", send_IsActive_PIR);

    // ::::::::::::::::: Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &send_Data, sizeof(send_Data));

    if (result == ESP_OK) {
      Serial.println("Massage Sending confirmed");
    }
    else {
      Serial.println("Sending error");
    }

    //read_and_get_PIR_sensor_data();
    delay(5000);
    Serial.println("Going to sleep now");
    digitalWrite(PIN_TO_GREEN_LED, HIGH);
    digitalWrite(PIN_TO_RED_LED, LOW);
    delay(1000);
    digitalWrite(PIN_TO_GREEN_LED, LOW);
    //esp_deep_sleep_start();

    Serial.println("-------------");
  }

}
// ________________________________________________________________________________



// ________________________________________________________________________________ VOID SETUP()
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();

  pinMode(ON_Board_LED, OUTPUT); //--> On Board LED port Direction output
  digitalWrite(ON_Board_LED, LOW); //--> Turn off Led On Board

  //ESP Wakeup Extranal
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_12, 1);

  // ---------------------------------------- Set Wifi to STA mode
  Serial.println();
  Serial.println("-------------");
  Serial.println("WIFI mode : STA");
  WiFi.mode(WIFI_STA);
  Serial.println("-------------");
  // ----------------------------------------

  // ---------------------------------------- Scan & Set the Wi-Fi channel.
  scan_and_set_Wifi_Channel();
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

  // ---------------------------------------- Once ESPNow is successfully Init, we will register for Send CB to get the status of Trasnmitted packet
  Serial.println();
  Serial.println("get the status of Trasnmitted packet");
  esp_now_register_send_cb(OnDataSent);
  // ----------------------------------------

  // ---------------------------------------- Register Peer
  Serial.println();
  Serial.println("Register peer");
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.encrypt = false;
  // ----------------------------------------

  // ---------------------------------------- Add Peer
  Serial.println();
  Serial.println("-------------");
  Serial.println("Starting to add Peers...");
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    Serial.println("-------------");
    return;
  }
  Serial.println("Adding Peers was successful.");
  Serial.println("-------------");
  // ----------------------------------------
  //PIR and LED
  pinMode(PIN_TO_PIR_SENSOR, INPUT);
  pinMode(PIN_TO_RED_LED, OUTPUT);
  pinMode(PIN_TO_GREEN_LED, OUTPUT);

}
// ________________________________________________________________________________

// ________________________________________________________________________________ VOID LOOP()
void loop() {

  read_and_get_PIR_sensor_data();

  //  unsigned long currentMillisScan = millis();
  //  if (currentMillisScan - previousMillisScan >= intervalScan) {
  //    previousMillisScan = currentMillisScan;
  //
  //    scan_and_set_Wifi_Channel();
  //  }
  // ----------------------------------------
}
