//
// A simple server implementation showing how to:
//  * serve static messages
//  * read GET and POST parameters
//  * handle missing pages / 404s
//
#include "Arduino.h"
#ifdef ESP32
#include "WiFi.h"
#include "AsyncTCP.h"
#elif defined(ESP8266)
#include "ESP8266WiFi.h"
#include "ESPAsyncTCP.h"
#endif
#include "ESPAsyncWebSrv.h"

#include "AsyncJson.h"
#include "ArduinoJson.h"

#include "Adafruit_NeoPixel.h"

#include "SPI.h"
#include "LoRa.h"

#ifdef __AVR__
  #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#define PIN_NEO 33
#define NUMPIXELS 1

Adafruit_NeoPixel pixels(NUMPIXELS, PIN_NEO, NEO_GRB + NEO_KHZ800);
//#define DELAY_NEO 500

#define PIN_CS 15
#define PIN_RESET 0
#define PIN_DIO0 27
#define PIN_DIO1 2
#define PIN_EN 32

#define LORA_TX_POWER 20
#define LORA_SPREADING_FACTOR 12

AsyncWebServer server(80);

const char* ssid = "NUSANTARA";
const char* password = "indonesia";

const char* PARAM_MESSAGE = "message";

void notFound(AsyncWebServerRequest *request) {
  String kirim;
  DynamicJsonDocument doc(1024);
  doc["resource"] = "not found";
  serializeJson(doc, kirim);
    //request->send(404, "text/plain", "Not found");
    //request->send(404, "application/json", "{\"message:\":\"Not found\"}");
    request->send(404, "application/json", kirim);
}

void setup() {

    Serial.begin(115200);
    // Initiate the LoRa Enable pin
    pinMode(PIN_EN, OUTPUT);
    // LoRa chip is Active High
    digitalWrite(PIN_EN, HIGH);
    delay(2000);

    LoRa.setPins(PIN_CS, PIN_RESET, PIN_DIO0);

    while (!LoRa.begin(920E6)) {
      Serial.println(".");
      delay(500);
    }

    //LoRa.setTxPower(LORA_TX_POWER, PA_OUTPUT_PA_BOOST_PIN);
    //LoRa.setSpreadingFactor(LORA_SPREADING_FACTOR);

    LoRa.setSyncWord(0xF3);
    Serial.println("LoRa Initializing OK!");

    #if defined (__AVR_ATtiny85__) && (F_CPU == 16000000)
      clock_prescale_set(clock_div_1);
    #endif

    // Initialize the RGB Neopixel LED
    pixels.begin();

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.printf("WiFi Failed!\n");
        return;
    }

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        //LoRa.beginPacket();
        //LoRa.print("hello ");
        //LoRa.endPacket(true);
        request->send(200, "text/plain", "Hello, world");
    });

    /*
    // Send a GET request to <IP>/get?message=<message>
    server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
        String message;
        if (request->hasParam(PARAM_MESSAGE)) {
            message = request->getParam(PARAM_MESSAGE)->value();
        } else {
            message = "No message sent";
        }
        request->send(200, "text/plain", "Hello, GET: " + message);
    });
    */

    /*
    // Send a POST request to <IP>/post with a form field message set to <message>
    server.on("/post", HTTP_POST, [](AsyncWebServerRequest *request){
        String message;
        //String params=request->getParam();
        if (request->hasParam(PARAM_MESSAGE, true)) {
            message = request->getParam(PARAM_MESSAGE, true)->value();
        } else {
            message = "No message sent";
        }
        request->send(200, "text/plain", "Hello, POST: " + message);
    });
    */

    /*
    AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler("/rest/endpoint",  [](AsyncWebServerRequest *request, JsonVariant &json) {
      JsonObject jsonObj = json.as<JsonObject>();
      request->send(200,"text/plain", "ale");
      // ...
    });
    */
    
      
    server.on("/rgb", HTTP_PUT, [](AsyncWebServerRequest * request){}, NULL,
      [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
      
      StaticJsonDocument<64> docReq;
      DeserializationError error = deserializeJson(docReq, data, len);

      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
      }
      
      //const char* name = docReq["name"];
      int r = docReq["r"];
      int g = docReq["g"]; // 255
      int b = docReq["b"]; // 255
      Serial.print("R: ");
      Serial.print(r);

      pixels.clear();

      pixels.setPixelColor(0, pixels.Color(r,g,b));

      pixels.show();
     
      String response;
      DynamicJsonDocument doc(1024);
      doc["rgb_led"] = "updated";
      serializeJson(doc, response);
      request->send(200, "application/json", response);
    });

    server.on("/lora-p2p-send", HTTP_POST, [](AsyncWebServerRequest * request){}, NULL,
      [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
      
        StaticJsonDocument<64> docReq;
        DeserializationError error = deserializeJson(docReq, data, len);

        if (error) {
          Serial.print("deserializeJson() failed: ");
          Serial.println(error.c_str());
          return;
        }
      
        const char* dataLoRa = docReq["data"];

        LoRa.beginPacket();
        LoRa.print(dataLoRa);
        LoRa.endPacket(true);

        String response;
        DynamicJsonDocument docRes(1024);
        docRes["lora-p2p"] = "sent";
        serializeJson(docRes, response);
        
        request->send(200, "application/json", response);
    });
    

     //server.addHandler(handler);
    server.onNotFound(notFound);

    server.begin();
}

void loop() {
  /*
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");

    // read packet
    while (LoRa.available()) {
      String LoRaData = LoRa.readString();
      Serial.print(LoRaData); 
    }

    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
  }
  */
}