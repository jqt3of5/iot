
#pragma SPARK_NO_PREPROCESSOR

#include <Particle.h>
#include <math.h>

#define R1 100000.0
#define MAX_A 4095.0
#define A 0.0008675874131
#define B 0.0001992667233
#define C 0.0000001323120683
#define SAMPLES 10

double avg_R0 = 0;
double avg_R1 = 0;
double avg_R2 = 0;

void setup()
{
  Particle.variable("R0",avg_R0);
  Particle.variable("R1",avg_R1);
  Particle.variable("R2",avg_R2);

  pinMode(D0, INPUT_PULLDOWN);
}

void loop()
{
  double temp_a0 = 0, temp_a1 = 0, temp_a2 = 0;
  for (int i = 0; i < SAMPLES; ++i)
  {
    temp_a0 += analogRead(A0);
    temp_a1 += analogRead(A1);
    temp_a2 += analogRead(A2);
    delay(500);
  }
  //Average the readings.
  temp_a0 = temp_a0/SAMPLES;
  temp_a1 = temp_a1/SAMPLES;
  temp_a2 = temp_a2/SAMPLES;

  //Convert the readings from analog values to an actual resistance.
  avg_R0 = temp_a0*R1/(MAX_A - temp_a0);
  avg_R1 = temp_a1*R1/(MAX_A - temp_a1);;
  avg_R2 = temp_a2*R1/(MAX_A - temp_a2);;

  //Convert the resistance into a tempurature using the precomputed Stienhart-Hart coefficients.
  double T0 = 1/(A + B*log(avg_R0) + C*log(avg_R0)*log(avg_R0)*log(avg_R0)) - 273.15;
  double T1 = 1/(A + B*log(avg_R1) + C*log(avg_R1)*log(avg_R1)*log(avg_R1)) - 273.15;
  double T2 = 1/(A + B*log(avg_R2) + C*log(avg_R2)*log(avg_R2)*log(avg_R2)) - 273.15;

  char data[100] = {0};
  sprintf(data, "{\"probeA\":%f, \"probeB\":%f, \"probeC\":%f}", T0, T1, T2);
  Particle.publish("thingspeak", data, 0, PRIVATE);
  Particle.publish("firebase", data, 0, PRIVATE);
  delay(10000);
}
