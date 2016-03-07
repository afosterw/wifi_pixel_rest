// NeoPixelFunLoop
// This example will move a trail of light around a series of pixels.  
// A ring formation of pixels looks best.  
// The trail will have a slowly fading tail.
// 
// This will demonstrate the use of the NeoPixelAnimator.
// It shows the advanced use an animation to control the modification and 
// starting of other animations.
// It also shows the normal use of animating colors.
// It also demonstrates the ability to share an animation channel rather than
// hard code them to pixels.
//

#include <NeoPixelBus.h>
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager

#define PIXEL_COUNT 16

const uint16_t PixelCount = PIXEL_COUNT; // make sure to set this to the number of pixels in your strip
const uint16_t PixelPin = 2;  // make sure to set this to the correct pin, ignored for Esp8266

NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart800KbpsMethod> strip(PixelCount, PixelPin);

ESP8266WebServer server(80);

void handleRoot() {
    server.send(200, "text/plain", "hello from esp8266!");
}

RgbColor temp_color;

String get_status() {
    String message = "{\"leds\":[";
    for (uint16_t i=0; i<PIXEL_COUNT; i++) {
        temp_color = strip.GetPixelColor(i);
        message += "{\"R\":" + String(temp_color.R) + ",\"G\":" + String(temp_color.G) + ",\"B\":" + String(temp_color.B) + "}";
        if(i < PIXEL_COUNT -1) {
            message += ",";
        }
    }
    message += "]}";
    return message;
}

void handle_led() {
    Serial.println("GET /led");
    if(server.method() == HTTP_GET) {
        Serial.println("GET /led");
        server.send(200, "application/json", get_status());
    } else {
        Serial.println("POST /led");
        for (uint8_t i=0; i<server.args(); i++){
            long number = strtol(server.arg(i).c_str(), NULL, 16);
            uint8_t r = number >> 16;
            uint8_t g = number >> 8 & 0xFF;
            uint8_t b = number & 0xFF;
            strip.SetPixelColor(server.argName(i).toInt(), RgbColor(r,g,b));
        }
        server.send(200, "application/json", get_status());
    }
}

void handle_clear() {
    if(server.args() > 0) {
        long number = strtol(server.arg(0).c_str(), NULL, 16);
        uint8_t r = number >> 16;
        uint8_t g = number >> 8 & 0xFF;
        uint8_t b = number & 0xFF;
        strip.ClearTo(RgbColor(r,g,b));
    } else {
        strip.ClearTo(RgbColor(0,0,0));
    }
    server.send(200, "application/json", get_status());
}

void handleNotFound(){
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET)?"GET":"POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i=0; i<server.args(); i++){
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    server.send(404, "text/plain", message);
}

void setup()
{
    Serial.begin(9600);
    //WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;
    //reset saved settings
    //wifiManager.resetSettings();
    
    wifiManager.autoConnect("AutoConnectAP");
    Serial.println("connected...yeey :)");

    strip.Begin();
    strip.Show();
    server.on("/", handleRoot);
    server.on("/led", handle_led);
    server.on("/led/clear", handle_clear);
    server.onNotFound(handleNotFound);
    server.begin();
    Serial.println("HTTP server started");
}


void loop()
{
    server.handleClient();
    strip.Show();
}
