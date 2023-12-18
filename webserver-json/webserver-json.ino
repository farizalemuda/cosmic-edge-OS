//
// A simple server implementation showing how to:
//  * serve static messages
//  * read GET and POST parameters
//  * handle missing pages / 404s
//

#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebSrv.h>

#include "AsyncJson.h"
#include "ArduinoJson.h"

AsyncWebServer server(80);

const char* ssid = "hahahaha";
const char* password = "hihihihi";

const char* PARAM_MESSAGE = "message";



void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

void setup() {

    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.printf("WiFi Failed!\n");
        return;
    }

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Hello, world");
    });

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

    AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler("/rest/endpoint",  [](AsyncWebServerRequest *request, JsonVariant &json) {
      JsonObject jsonObj = json.as<JsonObject>();
      request->send(200,"text/plain", "ale");
      // ...
    });

    server.on("/post2", HTTP_POST, [](AsyncWebServerRequest * request){}, NULL,
      [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
      
      StaticJsonDocument<16> doc;
      DeserializationError error = deserializeJson(doc, data, len);

      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
      }

      const char* name = doc["name"];
      
      Serial.print(name);
      /*
      for (size_t i = 0; i < len; i++) {
        Serial.write(data[i]);
      }
      */
      Serial.println();
 
      request->send(200,"text/plain", "ale");
  });

    //server.addHandler(handler);
    server.onNotFound(notFound);

    server.begin();
}

void loop() {
}