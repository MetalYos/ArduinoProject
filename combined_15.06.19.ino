#include <SoftwareSerial.h>
#include "HX711.h"
#include <Wire.h>

HX711 scale;

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
const char* openContainer = "o^";
const char* closeContainer = "c^";

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
      {
        calibration_weight = curr_weight;
        
        // open container num #
        String outputToSlave = container;
        outputToSlave += openContainer;
        Wire.beginTransmission(9);
        Wire.write(outputToSlave.c_str());
       Wire.endTransmission();
      }
      curr_weight = 0; 
    }
    counter++;
    if (too_much <= 0) 
    {
      //close container num #
        String outputToSlave = container;
        outputToSlave += closeContainer;
        Wire.beginTransmission(9);
        Wire.write(outputToSlave.c_str());
        Wire.endTransmission();
      Serial.print("Timeout, wanted weight was not recieved \n");
      return -100;
    }
  }   
  //close container num #
  String outputToSlave = container;
  outputToSlave += closeContainer;
  Wire.beginTransmission(9);
  Wire.write(outputToSlave.c_str());
  Wire.endTransmission();
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
  Wire.begin(); 

  // Sets the two pins as Outputs
  pinMode(stepPin,OUTPUT); 
  pinMode(dirPin,OUTPUT);

  scale.begin(data, clk);

  // Scale factor:
  // 1Kg cell: 2020 for reading in gms
  float factor = 2020 * 106.52 / 500;
  scale.set_scale(factor);                      // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();               // reset the scale to 0

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
        /*
        Serial.write("Container number:");
        Serial.println(container_int);
        Serial.write("\n");
        Serial.write("Prev Container number:");
        Serial.println(prevContainer);
        Serial.write("\n");
        */
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
