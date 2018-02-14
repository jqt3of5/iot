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
#include "tempProbe.h"
//#include <Adafruit_LSM303.h>
#include "DHT.h"

SYSTEM_MODE(SEMI_AUTOMATIC);

#define OPEN_REED D2
#define CLOSE_REED D3
#define DOOR_OVERRIDE D4
#define DOOR D7
#define DHTPIN D6    // what digital pin we're connected to

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

double _temp = 0, _humid = 0, _charge = 0;
bool _openSwitch = false;
bool _closeSwitch = false;
bool _asFarenheit = true;
bool _overridden = false;
int _autoShutTimeOut = 5;
int _timeToShut = 5;
bool _failedToShut = false;

void autoShut();
void error();
void refreshDisplay();

Timer _errorTimer(60 * 1000, error, true);
Timer _autoShutTimer(60 * 1000, autoShut, true);
Timer _refreshTimer(3 * 1000, refreshDisplay, false);

void ShowTimeToClose(int minutes);
void ShowError(int errorCode);
int toggle(String extra);

void error()
{
  _failedToShut = true;
}

void autoShut()
{
  _timeToShut -= 1;
  if (_timeToShut == 0)
  {
    _autoShutTimer.stopFromISR();
    toggle("");
  }
}

void displayString(const char * str)
{
  int len = strlen(str);
  for (int i = 0; i < len; ++i)
  {
    display.write(str[i]);
  }
}
void displayFloat(float value)
{
  char soc_str[16] = {0};
  sprintf(soc_str, "%f", value);

  int i = 0;
  for (i = 0; i < 15 && soc_str[i] != '.'; ++i)
  {
    display.write(soc_str[i]);
  }
  display.write(soc_str[i]);
  display.write(soc_str[i+1]);
}

void ShowBigTemp()
{
  display.setTextSize(2);
  display.setCursor(0,0);
  displayString("temp - F");

  display.setTextSize(5);
  display.setCursor(0,16);

  if (isnan(_temp))
  {
    displayString("--.-");
    return;
  }
  displayFloat(_temp);
}

void ShowBigHumid()
{
  display.setTextSize(2);
  display.setCursor(0,0);
  displayString("humid - %");

  display.setTextSize(5);
  display.setCursor(0,16);

  if (isnan(_humid))
  {
    displayString("--.-");
    return;
  }
  displayFloat(_humid);
}

void ShowTemp()
{
  display.setTextSize(2);
  display.setCursor(0,0);

  // Check if any reads failed and exit early (to try again).
  if (isnan(_humid) || isnan(_temp))
  {
    Serial.println("Failed to read from DHT sensor!");
    displayString("Error");
    return;
  }

  displayFloat(_temp);
  display.write('F');

  display.setCursor(64,0);
  displayFloat(_humid);
  display.write('%');
}

void ShowError(int errorCode)
{
  display.setTextSize(2);
  display.setCursor(0,20);
  displayString("ErrorCode: ");
  display.write(errorCode + '0');

}

void ShowOverridden()
{
  display.setTextSize(2);
  display.setCursor(0,20);
  displayString("Auto-close");
  display.setCursor(0,40);
  displayString("overridden");

}

void ShowTimeToClose(int minutes)
{
  display.setTextSize(2);
  display.setCursor(0,20);

  displayString("auto shut in ");
  display.write(minutes + '0');
}

void override()
{
  if (_openSwitch)
  {
    _overridden = !_overridden;
    if (_overridden)
    {
      _autoShutTimer.stopFromISR();
    }
    else
    {
      _timeToShut = _autoShutTimeOut;
      _autoShutTimer.startFromISR();
    }
  }
}

int toggle(String extra)
{
    digitalWrite(DOOR, HIGH);
    delay(500);
    digitalWrite(DOOR, LOW);
}

void setup()
{
  Particle.variable("humidity", _humid);
  Particle.variable("tempurature", _temp);
  Particle.variable("open", _openSwitch);
  Particle.variable("close", _closeSwitch);
  Particle.function("toggle", toggle);

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  // init done
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.display();

  dht.begin();

//  pinMode(D7, OUTPUT);

  pinMode(DOOR, OUTPUT);

  pinMode(OPEN_REED, INPUT_PULLDOWN);
  //attachInterrupt(OPEN_REED, open, CHANGE);

  pinMode(CLOSE_REED, INPUT_PULLDOWN);
  //attachInterrupt(CLOSE_REED, close, CHANGE);

  pinMode(DOOR_OVERRIDE, INPUT_PULLDOWN);
  attachInterrupt(DOOR_OVERRIDE, override, RISING);

  _humid = dht.readHumidity();
  _temp = dht.readTemperature(_asFarenheit);
  _closeSwitch = digitalRead(CLOSE_REED) == HIGH;
  _openSwitch = digitalRead(OPEN_REED) == HIGH;

  display.clearDisplay();
  refreshDisplay();

  _refreshTimer.start();

  Particle.connect();
}

void loop()
{
  _humid = dht.readHumidity();
  _temp = dht.readTemperature(_asFarenheit);

  bool closeSwitch = digitalRead(CLOSE_REED) == HIGH;
  bool openSwitch = digitalRead(OPEN_REED) == HIGH;

  if (closeSwitch && openSwitch)
  {
    _autoShutTimer.stop();
    _errorTimer.stop();
    _openSwitch = openSwitch;
    _closeSwitch = closeSwitch;
    return;
  }

  if (_openSwitch != openSwitch)
  {
    _failedToShut = false;
    _openSwitch = openSwitch;
    _overridden = false;
    //Door has opened
    if (_openSwitch)
    {
      //Enable AutoShut Timer
      _timeToShut = _autoShutTimeOut;
      _autoShutTimer.startFromISR();
      //Disable Error Timer
      _errorTimer.stopFromISR();
      Particle.publish("Door Opened");
    }
    //Door is closing
    else
    {
      //Enable Error Timer
      _errorTimer.startFromISR();
      Particle.publish("Door Closing");
    }
  }

  if (_closeSwitch != closeSwitch)
  {
    _failedToShut = false;
    _closeSwitch = closeSwitch;
    //Door has closed
    if (_closeSwitch)
    {
      //Disable Error Timer
      _errorTimer.stopFromISR();
      Particle.publish("Door Closed");
    }
    //Door is opening
    else
    {
      //Enable Error Timer
      _errorTimer.startFromISR();
      Particle.publish("Door Opening");
    }
  }

}

void refreshDisplay()
{
//  display.drawCircle(120,6,5,WHITE);
//  display.display();
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)

  //Causes flicker. Would be better to call on state transition
  display.clearDisplay();

  if (_openSwitch && _closeSwitch)
  {
    ShowError(1);
  }
  else if (_failedToShut)
  {
    ShowError(2);
  }
  else if (_openSwitch)
  {
    ShowTemp();
   if (_overridden)
   {
     ShowOverridden();
   }
   else
   {
     ShowTimeToClose(_timeToShut);
   }
  }
  else if (_closeSwitch)
  {
    ShowBigTemp();
  }
  else
  {
    ShowBigTemp();
  }

  display.display();
}
