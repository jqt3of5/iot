
#pragma SPARK_NO_PREPROCESSOR

#include <Particle.h>
#include <math.h>
#include "SparkFunMAX17043.h"

#define R1 100000.0
#define MAX_A 4095.0
#define A 0.0008675874131
#define B 0.0001992667233
#define C 0.0000001323120683
#define SAMPLES 10

char status[100] = {0};
void setup()
{
  Particle.variable("status", status);

  lipo.begin();
  lipo.quickStart();
}

void loop()
{
  double state_of_charge = lipo.getSOC();

  double temp_a0 = 0, temp_a1 = 0, temp_a2 = 0;
  for (int i = 0; i < SAMPLES; ++i)
  {
    temp_a0 += analogRead(A0);
    temp_a1 += analogRead(A1);
    temp_a2 += analogRead(A2);
    delay(100);
  }
  //Average the readings.
  temp_a0 = temp_a0/SAMPLES;
  temp_a1 = temp_a1/SAMPLES;
  temp_a2 = temp_a2/SAMPLES;

  //Convert the readings from analog values to an actual resistance.
  double avg_R0 = temp_a0*R1/(MAX_A - temp_a0);
  double avg_R1 = temp_a1*R1/(MAX_A - temp_a1);;
  double avg_R2 = temp_a2*R1/(MAX_A - temp_a2);;

  //Convert the resistance into a tempurature using the precomputed Stienhart-Hart coefficients.
  double T0 = 1/(A + B*log(avg_R0) + C*log(avg_R0)*log(avg_R0)*log(avg_R0)) - 273.15;
  double T1 = 1/(A + B*log(avg_R1) + C*log(avg_R1)*log(avg_R1)*log(avg_R1)) - 273.15;
  double T2 = 1/(A + B*log(avg_R2) + C*log(avg_R2)*log(avg_R2)*log(avg_R2)) - 273.15;

  sprintf(status, "{\"T0\":\"%f\",\"T1\":\"%f\",\"T2\":\"%f\",\"charge\":\"%f\"}", T0, T1, T2, state_of_charge);

  static bool chargeEventOnce = false;
  if (state_of_charge < 10)
  {
    if (!chargeEventOnce)
    {
      Particle.publish("low_charge", state_of_charge);
      chargeEventOnce = true;
    }
  }
  else
  {
    chargeEventOnce = false;
  }
  
/*  char data[100] = {0};
  sprintf(data, "{\"probeA\":%f, \"probeB\":%f, \"probeC\":%f}", T0, T1, T2);
  Particle.publish("temp", data, 0, PRIVATE);
  delay(10000);*/
}
