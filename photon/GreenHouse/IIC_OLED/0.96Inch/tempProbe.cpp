#include "tempProbe.h"
#include <Wire.h>

#define R1 100000.0
#define MAX_A 4095.0
#define A 0.0008675874131
#define B 0.0001992667233
#define C 0.0000001323120683

double readTemp(int pin)
{
  double temp = analogRead(pin);
  //Average the readings.
  double avg_R0 = temp*R1/(MAX_A - temp);
  return 1/(A + B*log(avg_R0) + C*log(avg_R0)*log(avg_R0)*log(avg_R0)) - 273.15;
}
