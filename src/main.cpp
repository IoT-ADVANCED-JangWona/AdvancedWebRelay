#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ConfigPortal8266.h>
#include <SPI.h>
#include <SSD1306.h>
#include <DHTesp.h>

char*               ssid_pfix = (char*)"CaptivePortal";
String              user_config_html = "";      
/*
 *  ConfigPortal library to extend and implement the WiFi connected IOT device
 *
 *  Yoonseok Hur
 *
 *  Usage Scenario:
 *  0. copy the example template in the README.md
 *  1. Modify the ssid_pfix to help distinquish your Captive Portal SSID
 *          char   ssid_pfix[];
 *  2. Modify user_config_html to guide and get the user config data through the Captive Portal
 *          String user_config_html;
 *  2. declare the user config variable before setup
 *  3. In the setup(), read the cfg["meta"]["your field"] and assign to your config variable
 *
 */

SSD1306             display(0x3c, 4, 5, GEOMETRY_128_32);

DHTesp              dht;
int                 interval = 2000;
unsigned long       lastDHTReadMillis = 0;
float               humidity = 0;
float               temperature = 0;

void readDHT22() {
    unsigned long currentMillis = millis();

    if(currentMillis - lastDHTReadMillis >= interval) {
        lastDHTReadMillis = currentMillis;

        humidity = dht.getHumidity();              // Read humidity (percent)
        temperature = dht.getTemperature();        // Read temperature as Fahrenheit
    }
}

void handleRoot(){
  String message = (webServer.method() == HTTP_GET)?"GET":"POST";
  message +=" "+webServer.uri()+"\n";
  for(uint8_t i=0; i<webServer.args(); i++){
    message +=" "+webServer.argName(i)+" : "+webServer.arg(i)+"\n";
  }
  message += "\nHello form ESP8266! IoT Training Week3 Assignment\n";
  webServer.send(200,"text/plain",message);
}

void Show_Temp() {
    String message = "Teamperature: " + String(temperature);
    webServer.send(200, "text/plain", message);
}

void Show_Humi() {
    String message = "Humidity: " + String(humidity);
    webServer.send(200, "text/plain", message);
}

void setup() {
    Serial.begin(115200);
    dht.setup(14, DHTesp::DHT22); // Connect DHT sensor to GPIO 14
    delay(1000);

    loadConfig();
    // *** If no "config" is found or "config" is not "done", run configDevice ***
    if(!cfg.containsKey("config") || strcmp((const char*)cfg["config"], "done")) {
        configDevice();
    }
    WiFi.mode(WIFI_STA);
    WiFi.begin((const char*)cfg["ssid"], (const char*)cfg["w_pw"]);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    // main setup
    Serial.printf("\nIP address : "); Serial.println(WiFi.localIP());

    if (MDNS.begin("miniwifi")) {
        Serial.println("MDNS responder started");
    }   
    webServer.begin();
    webServer.on("/", handleRoot); 
    webServer.on("/Show_Temp", Show_Temp);
    webServer.on("/Show_Humi", Show_Humi);
    webServer.on("/inline",[](){
    webServer.send(200,"text/plain","Hello from the inline function \n");
    });
}

void loop() {
    MDNS.update();
    webServer.handleClient();
    readDHT22();
    display.init(); 
    display.flipScreenVertically();
    display.drawString(10, 5, "Temperature: "+String(temperature));
    display.drawString(10, 15, "Humidity: "+String(humidity));
    Serial.printf("%.1f\t %.1f\n", temperature, humidity);
    display.display();
    delay(1000);
}