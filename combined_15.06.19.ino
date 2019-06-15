#include <SoftwareSerial.h>
#include "HX711.h"

HX711 scale;

//// defines pins numbers
//const int myRx = 6;
//const int myTx = 7;
//const int stepPin = 4; 
//const int dirPin = 5; 
//const int data = 3;
//const int clk = 2; 

// defines pins numbers
const int myRx = 7;
const int myTx = 8;
const int stepPin = 5; 
const int dirPin = 6; 
const int data = 3;
const int clk = 4; 
const int hall = 2;


// Constants
const int StepsPerRev = 200;
const int Pitch = 2; // (linear movement per rotation) mm
const int StepsPerMM = 100; // 100 steps
const int distanceBetweenContainers = 20; // mm
const int weightToleranceUnder100 = 1;
const int weightToleranceAbove100 = 5;
const double moveDeviation = 2;

// Variables
bool finishedReadInput = false;
bool execCommand = false;
char c = 0;
int totalSteps = 0;
String command;
bool started = false;
int prevContainer = 0;
int currentCommand = 0;
String container = "";

SoftwareSerial bluetooth(myRx, myTx);

void controlStepper(int dir, int numOfSteps)
{
  digitalWrite(dirPin, dir); // Enables the motor to move in a particular direction
  // Makes 200 pulses for making one full cycle rotation
  for(int x = 0; x < numOfSteps; x++)
  {
    digitalWrite(stepPin,HIGH); 
    delayMicroseconds(1000); 
    digitalWrite(stepPin,LOW); 
    delayMicroseconds(1000);
  }
}

int digit_to_int(char d)
{
  char str1[2];

  str1[0] = d;
  str1[1] = '\0';
  return (int) strtol(str1, NULL, 10);
}

double check_if_enough_weight(int n, double wanted_weight)
{
  double sum, curr_weight, val, calibration_weight;
  int too_much = 500;
  sum = curr_weight = val = calibration_weight =  0;
  int tolerance = wanted_weight * 0.1;
  int counter = 0;
  while ((wanted_weight > curr_weight + tolerance) || (wanted_weight < curr_weight - tolerance))
  {
    int i = 0;
    sum = 0;
    while (i < n) {
      val = ((scale.read() - scale.get_offset()) / scale.get_scale());
      sum += val - calibration_weight;
      i++;
    }
    too_much -= n;
    curr_weight = sum / n;
    Serial.print("Readings: ");
    Serial.print(sum / n, 3);
    Serial.print("gr \n");
    if (counter <= 2)
    {
      if (counter == 2)
      calibration_weight = curr_weight;
      curr_weight = 0;
    }
    counter++;
    if (too_much <= 0) 
    {
      Serial.print("Timeout, wanted weight was not recieved \n");
      return -100;
    }
  }   

  return curr_weight;
}

void goBack()
{
  controlStepper(LOW, totalSteps);
  totalSteps = 0;
  prevContainer = 0;
  container = "";
  Serial.println("went back");
  bluetooth.readString();
  finishedReadInput = false;
  execCommand = false;
  setup();
}

void setup() {
  Serial.begin(9600); // Console
  bluetooth.begin(9600); // HC-06

  // attachInterrupt(0, magnet_detect, LOW);//Initialize the intterrupt pin (Arduino digital pin 2)
  // Sets the two pins as Outputs
  pinMode(stepPin,OUTPUT); 
  pinMode(dirPin,OUTPUT);
  //  Serial.write("Beginning\n");
  //  Serial.println("HX711 Demo");
  //
  //  Serial.println("Initializing the scale");
  // parameter "gain" is ommited; the default value 128 is used by the library
  // HX711.DOUT  - pin #A1
  // HX711.PD_SCK - pin #A0
  scale.begin(data, clk);
  //
  //  Serial.print("Raw ave(20): \t\t");
  //  Serial.println(scale.read_average(20));   // print the average of 20 readings from the ADC

  // Scale factor:
  // 1Kg cell: 2020 for reading in gms
  // 50kg cells: 19150 for reading in kg
  float factor = 2020 * 106.52 / 500;
  scale.set_scale(factor);                      // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();               // reset the scale to 0
  //
  //  Serial.println("\nAfter setting up the scale:");
  //
  //  Serial.print("Raw: \t\t\t");
  //  Serial.println(scale.read());                 // print a raw reading from the ADC
  //
  //  Serial.print("Raw ave(20): \t\t");
  //  Serial.println(scale.read_average(20));       // print the average of 20 readings from the ADC
  //
  //  Serial.print("Raw ave(5) - tare: \t");
  //  Serial.println(scale.get_value(5));   // print the average of 5 readings from the ADC minus the tare weight, set with tare()
  //
  //  Serial.print("Calibrated ave(5): \t");
  //  Serial.println(scale.get_units(5), 1);        // print the average of 5 readings from the ADC minus tare weight, divided
  //  // by the SCALE parameter set with set_scale
  //
  //  Serial.println("\nReadings:");
  //      int i = 0;
  //      double sum = 0;
  //    while (i < 20) {
  //      double val = ((scale.read() - scale.get_offset()) / scale.get_scale());
  //      sum += val;
  //      i++;
  //    }
  //    
  //    Serial.print("Readings: ");
  //    Serial.print(sum / 20, 3);
  //    Serial.print("gr \n");

  finishedReadInput = false;
  execCommand = false;
  command ="";
  currentCommand = 0;

  Serial.write("Begin\n\n");
  while(digitalRead(hall) == HIGH)
  {
    Serial.println("Waiting for bowl!");
  }
}


void loop() { 
  container = "";
  String weight = "";
  while (bluetooth.available())
  {   
    if (!finishedReadInput)
    {
      command = bluetooth.readString();
      for (int i = 0; i < command.length(); i++)
      Serial.write(command[i]);
      Serial.write("\n");

      if (command[command.length() - 3] == '^')
      {
        execCommand = true;
        finishedReadInput = true;
        currentCommand = 0;
      }
    }
  } 

  if (execCommand)
  {
    if ( digitalRead(hall) == LOW )
    {
      char dir = command[0+currentCommand];
      container = command[1+currentCommand];
      weight += command[3+currentCommand];
      weight += command[4+currentCommand];
      weight += command[5+currentCommand];
      Serial.print("weight to funcion");
      Serial.println(weight);
      Serial.print("\n");
      double w_weight = weight.toDouble();

      int container_int = 0;
      //the difference between stations. container_intulated by subtracting the previous position from the current position.
      int containerDiff = 0;
      if (dir == 'f')
      {
        container_int = container.toInt();
        Serial.write("Container number:");
        Serial.println(container_int);
        Serial.write("\n");
        Serial.write("Prev Container number:");
        Serial.println(prevContainer);
        Serial.write("\n");
        containerDiff = container_int - prevContainer;
        int numSteps = (containerDiff * distanceBetweenContainers) * StepsPerMM;
        controlStepper(HIGH, numSteps);
        //*********************************************************
        char toPrint[20] = {0};
        Serial.write("\nNumber of steps: ");
        itoa(numSteps,toPrint,10);
        Serial.write(toPrint);
        Serial.write("\n");
        //*********************************************************
        totalSteps += numSteps;
        prevContainer = container_int; 
      }
      else
      {
        goBack();
      }
      
      // Wait for command to continue
      //    if (command[currentCommand] == 'b')
      //    {
      //      setup();
      //    }
      Serial.print("weight to funcion ");
      Serial.println(weight);
      Serial.print("\n");
      double recieved_weight = check_if_enough_weight(20, w_weight);
      if (recieved_weight < 0)
      {
        //close container, go back and update amounts in containers
        finishedReadInput = false;
        execCommand = false;
      }
      else 
      {
        Serial.print("Recieved weight: ");
        Serial.print(recieved_weight, 3);
        Serial.print("gr \n");
        scale.tare();
        //close hatch for good
      }
      
      Serial.write("\nEnd Loop\n");

      bluetooth.print(container);
      bluetooth.print('$');
      bluetooth.print(recieved_weight, 3);
      bluetooth.print(';');
      
      currentCommand += 7;
    }
    //bowl is not on the scale, pin shows HIGH
    else
    {
      String answer = "";
      Serial.println("else");
      while (bluetooth.available())
      {
        answer = bluetooth.readString();
        //Serial.println(answer);
        if (answer[0] == 'y')
        {
          Serial.println("I'm in");
          while(digitalRead(hall) == HIGH);
          Serial.println("I'm done");
        }
        else
        {
          goBack();
        }  
      } 
    }  
  } 
}
