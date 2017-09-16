/*********************************************************************
This is an example for our Monochrome OLEDs based on SSD1306 drivers

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/category/63_98

This example is for a 128x64 size display using I2C to communicate
3 pins are required to interface (2 I2C and one reset)

Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution
*********************************************************************/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "SparkFunMAX17043.h" // Include the SparkFun MAX17043 library
#include "tempProbe.h"

#include "DHT.h"

SYSTEM_MODE(SEMI_AUTOMATIC);

#define DHTPIN A0    // what digital pin we're connected to

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);


#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

int _tempIndex = 0;
double _tempHistory[100] = {0};
int _humidIndex = 0;
double _humidHistory[100] = {0};
int _chargeIndex = 0;
double _chargeHistory[100] = {0};

double _temp = 0, _humid = 0, _charge = 0;
bool _asFarenheit = true;
void displayFloat(float value);

void setup()   {
  Serial.begin(9600);
  delay(200);

  Particle.variable("humidity", _humid);
  Particle.variable("tempurature", _temp);
  Particle.variable("charge", _charge);
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  // init done
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.display();

  dht.begin();
   // Set up the MAX17043 LiPo fuel gauge:
	lipo.begin(); // Initialize the MAX17043 LiPo fuel gauge

	// Quick start restarts the MAX17043 in hopes of getting a more accurate
	// guess for the SOC.
	lipo.quickStart();

  Particle.connect();
}

void loop()
{

  display.drawCircle(120,6,5,WHITE);
  display.display();
    // Reading temperature or humidity takes about 250 milliseconds!
   // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
   _humid = dht.readHumidity();
   _humidHistory[_humidIndex] = _humid;
   _humidIndex = (_humidIndex + 1)%100;

   delay(2000);
   //float t = dht.readTemperature();
   // Read temperature as Fahrenheit (isFahrenheit = true)
   _temp = dht.readTemperature(_asFarenheit);
   _tempHistory[_tempIndex] = _temp;
   _tempIndex = (_tempIndex + 1)%100;

   // Check if any reads failed and exit early (to try again).
   if (isnan(_humid) || isnan(_temp)) {
     Serial.println("Failed to read from DHT sensor!");
     return;
   }

   //double voltage = lipo.getVoltage();
	 _charge = lipo.getSOC();
   _chargeHistory[_chargeIndex] = _charge;
   _chargeIndex = (_chargeIndex + 1)%100;

   double probe = readTemp(A1);
   if (_asFarenheit)
   {
     probe = probe * 1.8 + 32;
   }

   // Compute heat index in Fahrenheit (the default)
   //float hif = dht.computeHeatIndex(f, h);
   // Compute heat index in Celsius (isFahreheit = false)
   //float hic = dht.computeHeatIndex(t, h, false);

  display.clearDisplay();
  display.setCursor(0,0);
  displayFloat(_charge);
  display.write('%');

  display.setCursor(0,20);
  displayFloat(_temp);

  display.setCursor(0,40);
  displayFloat(_humid);
  display.write('%');

  display.setCursor(64,20);
  displayFloat(probe);

  display.display();
  delay(2000);
}

void displayFloat(float value)
{
  char soc_str[16] = {0};
  sprintf(soc_str, "%f", value);

  display.write(soc_str[0]);
  display.write(soc_str[1]);
  display.write(soc_str[2]);
  display.write(soc_str[3]);
}

void drawGraph(float [] data, int start, int count)
{
  for (int i = 0; i < count; ++i)
  {

  }
}
