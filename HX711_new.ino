#include "HX711.h"

HX711 scale;

void setup() {
  Serial.begin(38400);
  Serial.println("HX711 Demo");

  Serial.println("Initializing the scale");
  // parameter "gain" is ommited; the default value 128 is used by the library
  // HX711.DOUT  - pin #A1
  // HX711.PD_SCK - pin #A0
  scale.begin(3, 2);

  Serial.print("Raw ave(20): \t\t");
  Serial.println(scale.read_average(20));   // print the average of 20 readings from the ADC

  // Scale factor:
  // 1Kg cell: 2020 for reading in gms
  // 50kg cells: 19150 for reading in kg
  float factor = 2020 * 104.15 /500;
  scale.set_scale(2020.f);                      // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();               // reset the scale to 0

  Serial.println("\nAfter setting up the scale:");

  Serial.print("Raw: \t\t\t");
  Serial.println(scale.read());                 // print a raw reading from the ADC

  Serial.print("Raw ave(20): \t\t");
  Serial.println(scale.read_average(20));       // print the average of 20 readings from the ADC

  Serial.print("Raw ave(5) - tare: \t");
  Serial.println(scale.get_value(5));   // print the average of 5 readings from the ADC minus the tare weight, set with tare()

  Serial.print("Calibrated ave(5): \t");
  Serial.println(scale.get_units(5), 1);        // print the average of 5 readings from the ADC minus tare weight, divided
            // by the SCALE parameter set with set_scale

  Serial.println("\nReadings:");
}

void loop() {
  int t, i, n, T;
  double val, sum, sumsq, mean;
  float stddev;
  
  n = 20;
  t = millis();
  i = sum = sumsq = 0;
  while (i<n) {
    val = ((scale.read() - scale.get_offset()) / scale.get_scale());
    sum += val;
    sumsq += val * val;
    i++;
  }
  t = millis() - t;
  mean = sum / n;
  stddev = sqrt(sumsq / n - mean * mean);
  Serial.print("Mean, Std Dev of "); Serial.print(i); Serial.print(" readings:\t");
  Serial.print(sum / n, 3); Serial.print("\t"); Serial.print(stddev, 3);
  // Note: 2 sigma is 95% confidence, 3 sigma is 99.7%
  Serial.print("\nTime taken:\t"); Serial.print(float(t)/1000, 3); Serial.println("Secs\n");

  /*
  scale.power_down();             // put the ADC in sleep mode
  delay(5000);
  scale.power_up();
  */
}
