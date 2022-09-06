/***************************************************************************
  This is a library for the BMP280 humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BMP280 Breakout
  ----> http://www.adafruit.com/products/2651

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include <WiFi.h>
#include <Adafruit_AHTX0.h>
#include <WebServer.h>

#define BMP_SCK  (13)
#define BMP_MISO (12)
#define BMP_MOSI (11)
#define BMP_CS   (10)

Adafruit_AHTX0 aht;


Adafruit_BMP280 bmp; // I2C
//Adafruit_BMP280 bmp(BMP_CS); // hardware SPI
//Adafruit_BMP280 bmp(BMP_CS, BMP_MOSI, BMP_MISO,  BMP_SCK);
int LED_BUILTIN = 2;
float temperature, humidity, pressure, altitude;

/*Webserver settings*/
const char* ssid = "";  // Enter SSID here
const char* password = "";  //Enter Password here

WebServer server(80);   

void setup() {
  Serial.begin(9600);
  pinMode (LED_BUILTIN, OUTPUT); 
  while ( !Serial ) delay(100);   // wait for native usb
  Serial.println(F("BMP280 test"));
  unsigned status;
  //status = bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID);
  status = bmp.begin();
  if (!status) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
    Serial.print("SensorID was: 0x"); Serial.println(bmp.sensorID(),16);
    Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    Serial.print("        ID of 0x60 represents a BME 280.\n");
    Serial.print("        ID of 0x61 represents a BME 680.\n");
    while (1) delay(10);
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

   if (! aht.begin()) {
    Serial.println("Could not find AHT? Check wiring");
    while (1) delay(10);
  }
  Serial.println("AHT10 or AHT20 found");
   WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  Serial.print(".");
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());
  server.begin();
  Serial.println("HTTP server started");
  }
}

void handle_OnConnect() {
   sensors_event_t humidity1, temp1;
    aht.getEvent(&humidity1, &temp1);// populate temp and humidity objects with fresh data
    temperature = temp1.temperature;
    humidity = humidity1.relative_humidity;
    pressure = int(bmp.readPressure()/100 + 31.5);
    altitude = bmp.readAltitude(1013.25)+ 31.5;
  server.send(200, "text/html", SendHTML(temperature,humidity,pressure,altitude)); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

void loop() {
    server.handleClient();
    server.on("/", handle_OnConnect);
    server.onNotFound(handle_NotFound);
    digitalWrite(LED_BUILTIN, HIGH);
    sensors_event_t humidity, temp;
    aht.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
    Serial.print("Temperature: "); Serial.print(temp.temperature); Serial.println(" degrees C");
    Serial.print("Humidity: "); Serial.print(humidity.relative_humidity); Serial.println("% rH");
    Serial.print(F("Pressure = "));
    Serial.print(int(bmp.readPressure()/100 + 31.5)); /*change 31 into your exsact altitude / 10 to get hPa*/
    Serial.println(" hPa");
    Serial.print(F("Approx altitude = "));
    Serial.print(bmp.readAltitude(1013.25)+ 31.5); /* Adjusted to local forecast! */
    Serial.println(" m");
    Serial.println("*****************************************************************************");
    Serial.println("*****************************************************************************");
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
}

String SendHTML(float temperature,float humidity,float pressure,float altitude){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<meta charset=\"UTF-8\">\n";
  ptr +="<title>Sobna vremenska postaja xD</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}\n";
  ptr +="p {font-size: 24px;color: #444444;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<div id=\"webpage\">\n";
  ptr +="<h1>*******Sobna vremenska postaja*******</h1>\n";
  ptr +="<p>Temperatura: ";
  ptr +=temperature;
  ptr +="&deg;C</p>";
  ptr +="<p>Vlaga: ";
  ptr +=humidity;
  ptr +="%</p>";
  ptr +="<p>Pritisk: ";
  ptr +=pressure;
  ptr +="hPa</p>";
  ptr +="<p>Relativna nadmorska višina: ";
  ptr +=altitude;
  ptr +="m</p>";
  ptr +="</div>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}
