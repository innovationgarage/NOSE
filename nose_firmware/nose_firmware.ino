#define VERSION "v2"
#include <Wire.h>        // I2C library
#include <ESP8266WiFi.h> // ESP8266 WiFi library
#include <ThingSpeak.h>  // ThingSpeak library
#include "I2Cbus.h"      // I2C bus clear library
#include "ccs811.h"      // CCS811 library
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <BlynkSimpleEsp8266.h>
#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <WS2812FX.h>
#include "credentials.h"

#define PIN D8
#define NUMws2812fx 5
#define DHTPIN D5     // Digital pin connected to the DHT sensor 
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

WS2812FX ws2812fx = WS2812FX(NUMws2812fx, PIN, NEO_RGB + NEO_KHZ800);

// Uncomment the type of sensor in use:
#define DHTTYPE    DHT11     // DHT 11
//#define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

#define LED_PIN    D4    // GPIO2 == D4 == standard BLUE led available on most NodeMCU boards (LED on == D4 low)
#define led_init() pinMode(LED_PIN, OUTPUT)
#define led_on()   digitalWrite(LED_PIN, LOW)
#define led_off()  digitalWrite(LED_PIN, HIGH)
#define led_tgl()  digitalWrite(LED_PIN, (HIGH+LOW)-digitalRead(LED_PIN) );

// Wrapper for blinking 'blink' times, each blink (off+on) taking `ms` ms.
void led_blink(int blink, int ms ) {
  for ( int i = 0; i < 2 * blink; i++ ) {
    led_tgl();
    delay(ms / 2);
  }
}

WiFiClient  client;
CCS811      ccs811(D3); // nWAKE on D3

DHT_Unified dht(DHTPIN, DHTTYPE);
unsigned long delayMS;

// Keep track of "last known correct values" for each of the ThingSpeak fields (except status)
int   Field1_eCO2     = 400;    // ppm
int   Field2_eTVOC    = 0;      // ppb
//    Field3 ERROR_ID & STATUS
float Field4_Resist   = 100000; // Ω
float Field5_T        = 25.0;   // °C
float Field6_H        = 50.0;   // %RH
//    Field7  TH status combined
int   Field8_errcount = 0;
