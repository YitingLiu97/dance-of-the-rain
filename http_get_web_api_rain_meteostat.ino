/*

  Yiting Liu's weather project that translates rain data into light dance.

  This sketch gets the historical weather data from api.meteostat.net, and parse the content into precipitation and rainy days, which are then mapped towards different rhythmic light on a LED strip.


  Wiring (Arduino IoT33):
  LED strip on Pin 7

*/
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
#define PIN        7 // On Trinket or Gemma, suggest changing this to 1
#define NUMPIXELS 16 // Popular NeoPixel ring size
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#define DELAYVAL 50 // Time (in milliseconds) to pause between pixels

#include <ArduinoHttpClient.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h"
#include <ArduinoJson.h>

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
String API = API_KEY;

const char serverAddress[] = "api.meteostat.net";  // server address
int port = 443;
String history = "/v1/history/";
String monthly = "monthly?";
String startDate = "2017-01";
String endDate = "2017-12";
String stationID = "72503"; //NYC LGA

WiFiSSLClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, port);
int status = WL_IDLE_STATUS;

void setup() {

  Serial.begin(9600);
  connectWifi();
  pixels.begin();

}



void loop() {
  getData();
  delay(DELAYVAL);
}

void getData() {

  // assemble the path for the GET message:
  String path = history + monthly + "station=" + stationID + "&start=" + startDate + "&end=" + endDate + "&key=" + API;
  String contentType = "application/json";

  // send the GET request
  Serial.println("making GET request");
  Serial.println(path);

  client.get(path);

  // read the status code and body of the response
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();

  exampleData(response);


}

//using JSON assistant for the weather data rather than HTTP

int maxRain = 0;
int minRain = 0;
int maxRaindays = 0;
int minRaindays = 0;

void exampleData(String response) {

  const size_t capacity = JSON_ARRAY_SIZE(12) + JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + 12 * JSON_OBJECT_SIZE(10) + 2050;
  DynamicJsonDocument doc(capacity);

  Serial.print("response: ");
  Serial.println(response);

  deserializeJson(doc, response);

  JsonObject obj = doc.as<JsonObject>();

  const char* meta_source = doc["meta"]["source"]; // "National Oceanic and Atmospheric Administration, Deutscher Wetterdienst"

  JsonArray data = obj["data"];

  int rain[12] = {};
  int raindays[12] = {};
  int lightValue = 0;


  for (int i = 0; i < data.size(); i++) {

    rain[i] = data[i]["precipitation"];
    raindays[i] = data[i]["raindays"];

    //    Serial.print("rain: ");
    //    Serial.println(rain[i]);
    //    Serial.print("raindays: ");
    //    Serial.println(raindays[i]);

    if (rain[i] > maxRain) {
      maxRain = rain[i];
    }
    if (rain[i] < minRain) {
      minRain = rain[i];

    }
    //    Serial.print("maxRain: ");
    //    Serial.println(maxRain);
    //    Serial.print("minRain: ");
    //    Serial.println(minRain);

    if (raindays[i] > maxRaindays) {
      maxRaindays = raindays[i];
    }
    if (raindays[i] < minRaindays) {
      minRaindays = raindays[i];

    }
    Serial.print("maxRaindays: ");
    Serial.println(maxRaindays);
    Serial.print("minRaindays: ");
    Serial.println(minRaindays);

    int frequency = (maxRaindays - minRaindays) % raindays[i];
    int lightValue = map(rain[i], minRain, maxRain, 0, 255);
    Serial.print("frequency: ");
    Serial.println(frequency);
    Serial.print("lightValue: ");
    Serial.println(lightValue);
    showPixel(lightValue, raindays[i], frequency);
  }
}

//next step is to convert the data into the led strip presentation
//the rain would be the max and min of the 0 and 255 - determining the color blue
// the raindays would be the speed
void showPixel(int lightValue, int raindays, int skipValue) {
  pixels.clear();

  if (skipValue != 0) {
    for (int i = 0; i < NUMPIXELS; i += skipValue) { // For each pixel...

      // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
      pixels.setPixelColor(i, pixels.Color(0, 0, lightValue));
      pixels.setBrightness(lightValue);

      pixels.show();   // Send the updated pixel colors to the hardware.

      delay(raindays * DELAYVAL); // Pause before next pass through loop
      //make it flow backwards?
      //convert it into musical stuff?

    }
  }
}


void connectWifi() {

  while (!Serial);
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);     // print the network name (SSID);

    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);
  }

  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

}
