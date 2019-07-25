/*
 * Written By Donnie Plumly of THink, Make, Repeat.
   Using an Arduino Uno on both ends

  Transmitter wiring - 3 pin from left to right     https://smile.amazon.com/gp/product/B017AYH5G0/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1
  ATAD - Pin 3
  Vcc - 5v
  Gnd - Gnd

  DHT22 wiring - 3 pin from left to right    https://www.adafruit.com/product/385
  Vcc - 5v
  DATA - Pin 8
  N/A
  Gnd - Gnd

  Adafruit Seesaw soil sensor from left to right      https://www.adafruit.com/product/4026
  Gnd - Gnd
  5v - 5v
  SDA - A4
  SCL - A5

  Adafruit VEML6070 from left to right     https://www.adafruit.com/product/2899
  Vin - 5v
  Gnd - Gnd
  SCL - A5
  SDA - A4
  

*/
#include "VirtualWire.h"
#include "LowPower.h"
#include "DHT.h"
#include "Adafruit_seesaw.h"
#include "Wire.h"
#include "Adafruit_VEML6070.h"

/****************************************************
  DHT Globals
****************************************************/
#define DHTPIN 8
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

struct package
{
  float temperature;
  float humidity;
  float soilTemp;
  //int soilCap;
  int uvLight;
};
/****************************************************
  Transmitter Globals
****************************************************/
const int led_pin = 13;
const int transmit_pin = 3;

typedef struct package Package;
Package data;

/****************************************************
  Soil Sensor Globals
****************************************************/
Adafruit_seesaw ss;

/****************************************************
  UV Sensor Globals
****************************************************/
Adafruit_VEML6070 uv = Adafruit_VEML6070();

void setup()
{
  Serial.begin(9600);
  Serial.println("Transmitter Ready");

  // Initialise the IO and ISR
  vw_set_tx_pin(transmit_pin);
  vw_set_ptt_inverted(true); // Required for DR3100
  vw_setup(500);       // Bits per sec
  pinMode(led_pin, OUTPUT);


  /****************************************************
    Soil Sensor Setup I2C
  ****************************************************/
  Serial.println("seesaw Soil Sensor example!");

  if (!ss.begin(0x36)) {
    Serial.println("ERROR! seesaw not found");
    while (1);
  } else {
    Serial.print("seesaw started! version: ");
    Serial.println(ss.getVersion(), HEX);
  }

  /****************************************************
    UV Sensor Setup I2C
  ****************************************************/
  Serial.println("VEML6070 Test");
  uv.begin(VEML6070_1_T);  // pass in the integration time constant
}



void loop()
{
  digitalWrite(led_pin, HIGH); // Flash a light to show transmitting
  Serial.println("Sending Package");
  dhtData();
  soilData();
  uvData();
  vw_send((uint8_t *)&data, sizeof(data));
  vw_wait_tx(); // Wait until the whole message is gone
  digitalWrite(led_pin, LOW);
  Serial.println("Sent Package");
  delay(2000);
  //sleepForTwoMinutes();
}

/****************************************************
    Read Ambient Temp and Humidity
****************************************************/
void dhtData()
{
  dht.begin();
  delay(2000);
  data.humidity = dht.readHumidity();
  //data.temperature = dht.readTemperature();
  data.temperature = dht.readTemperature(true); // reads in f??? try at work
  Serial.print("Ambient Temperature: "); Serial.println(data.temperature);
  Serial.print("Humidity: "); Serial.println(data.humidity);

}
/****************************************************
    Read Soil Temp
****************************************************/
void soilData()
{
  data.soilTemp = ss.getTemp();
  uint16_t capread = ss.touchRead(0);

  //data.soilTemp = tempC;
  //data.soilCap = capread;

  Serial.print("Soil Temperature: "); Serial.print(data.soilTemp); Serial.println("*C");
  Serial.print("Capacitive: "); Serial.println(capread);
  delay(100);
}
/****************************************************
    Read UV Light
****************************************************/
void uvData() {
  data.uvLight = uv.readUV();
  Serial.print("UV light level: "); Serial.println(data.uvLight);
  delay(1000);
}
/****************************************************
    Sleep for an hour
****************************************************/
void sleepForTwoMinutes() {
  for (int i = 0; i < 3600; i++)
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
}








