/***************************************************

  Receiver wiring - 4 pin module from left to right
  Vcc - 5v
  DATA - Pin 2
  N/A
  Gnd - Gnd

  Transmitter wiring - 3 pin module from left to right
  Vcc - 5v
  DATA - Pin 3
  Gnd - Gnd

  Seeedstudio 2.8'' TFT Touch Shield v2.0 Pinout
  D0  NOT USED
  D1  NOT USED
  D2  Receiver Data
  D3  ATAD Transmitter Data
  D4  TF_CS
  D5  TFT_CS
  D6  TFT_DC
  D7  BACKLIGHT(Selectable)
  D8  DHT22 Data
  D9  NOT USED
  D10 NOT USED
  D11 SPI_MOSI
  D12 SPI_MISO
  D13 SPI_SCK

  USE ISCP PINS
  RESET to RESET

  489F48
 ****************************************************/
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "VirtualWire.h"
#include "SPI.h"
#include "SD.h"
#include "Wire.h"
#include "Adafruit_VEML6070.h"
#include <TFTv2.h>


/****************************************************
  TFT LCD Globals
****************************************************/
#define TFT_DC 6
#define TFT_CS 5
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);


/****************************************************
  SD Globals
****************************************************/
File myFile;
const int chipSelect = 4; // Pin 4 to write to SD


/****************************************************
  Sensor Globals
****************************************************/
const int receive_pin = 2;
//char temperatureChar[10];
//char humidityChar[10];
//char soilTempChar[10];

struct package
{
  float temperature = 0.0;
  float humidity = 0.0;
  float soilTemp = 0.0;
  int uvLight = 0;
};

typedef struct package Package;
Package data;

void setup()
{
  /****************************************************
     Start TFT and test for debugging
   ****************************************************/
  Serial.begin(9600);
  tft.begin();

  // read diagnostics (optional but can help debug problems)
  /*
    uint8_t x = tft.readcommand8(ILI9341_RDMODE);
    Serial.print("Display Power Mode: 0x"); Serial.println(x, HEX);
    x = tft.readcommand8(ILI9341_RDMADCTL);
    Serial.print("MADCTL Mode: 0x"); Serial.println(x, HEX);
    x = tft.readcommand8(ILI9341_RDPIXFMT);
    Serial.print("Pixel Format: 0x"); Serial.println(x, HEX);
    x = tft.readcommand8(ILI9341_RDIMGFMT);
    Serial.print("Image Format: 0x"); Serial.println(x, HEX);
    x = tft.readcommand8(ILI9341_RDSELFDIAG);
    Serial.print("Self Diagnostic: 0x"); Serial.println(x, HEX);
    Serial.println(F("Done!"));
  */
  // Set rotation of screen to horizontal
  tft.setRotation(1);


  /****************************************************
     Start radio communication
   ****************************************************/
  Serial.println("Radio Ready");

  // Initialise the IO and ISR
  vw_set_rx_pin(receive_pin);
  vw_setup(500);   // Bits per sec
  vw_rx_start();   // Start the receiver PLL running


  /****************************************************
    Start SD Card
  ****************************************************/
  pinMode(chipSelect, OUTPUT);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
}


void loop()
{
  Serial.println("Start loop");
  dataReceived();
  Serial.println("End loop");
  delay(4000);
}


/****************************************************
    Send data to hub
****************************************************/
void dataReceived ()
{
  Serial.println("Start sending data function");
  uint8_t buf[sizeof(data)];
  uint8_t buflen = sizeof(data);

  delay(200);

  if (vw_have_message())  // Is there a packet for us?
  {
    Serial.println("begin if message available");
    vw_get_message(buf, &buflen);
    memcpy(&data, &buf, buflen);

    // convert to Fahrenheit
    //float temperature = (data.temperature * 9 + 2) / 5 + 32; //may not need  seetrans file

    // temp data serial printing
    Serial.print(data.temperature); Serial.println(" f");
    // humid data serial printing
    Serial.print(data.humidity); Serial.println(" h");
    Serial.println(data.soilTemp); Serial.println(" st");
    Serial.println(data.uvLight); Serial.println(" uv");

    // print all data to screen
    tft.fillScreen(ILI9341_BLACK);
    unsigned long start = micros();
    tft.setTextColor(ILI9341_GREEN); tft.setTextSize(2);

    // Grid
    Tft.drawRectangle(10, 10, 145, 105, GREEN);
    Tft.drawRectangle(10, 125, 145, 105, GREEN);
    Tft.drawRectangle(165, 10, 145, 105, GREEN);
    Tft.drawRectangle(165, 125, 145, 105, GREEN);

    tft.setCursor(55, 52.5);
    tft.print(data.temperature, 1);
    tft.setCursor(20, 77.5);
    tft.print("Fahrenheit");

    tft.setCursor(200, 52.5);
    tft.print(data.humidity, 1);
    tft.setCursor(190, 77.5);
    tft.print("Humidity");

    tft.setCursor(55, 170);
    tft.print(data.soilTemp, 1);
    tft.setCursor(30, 195);
    tft.print("Soil Temp");

    tft.setCursor(230, 170);
    tft.print(data.uvLight);
    tft.setCursor(190, 195);
    tft.print("UV Light");

    delay(4000);
    Serial.println("end if statement");

    dataLogger(); // Enter dataLogger function to log data to SD card

    return micros() - start;
  }
}

/****************************************************
    Log data to SD card
****************************************************/
void dataLogger ()
{
  Serial.println("Entering Data Logger");

  String dataString = "";

  dataString += String(data.humidity);
  dataString += String("%");
  dataString += ", ";
  dataString += String(data.temperature);
  dataString += String("F");

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile)
  {
    dataFile.println(dataString);
    Serial.println("Printing data to file");

    dataFile.close();
    // print to the serial port too:
    Serial.println(dataString);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }
}
/****************************************************
    Grid Pattern
****************************************************/
unsigned long testRects(uint16_t color) {
  unsigned long start;
  int           n, i, i2,
                cx = tft.width()  / 2,
                cy = tft.height() / 2;

  //tft.fillScreen(ILI9341_BLACK);
  n     = min(tft.width(), tft.height());
  start = micros();
  for (i = 1; i < n; i += 6) {
    i2 = i / 2;
    tft.drawRect(cx - i2, cy - i2, i, i, color);
  }

  return micros() - start;
}


































