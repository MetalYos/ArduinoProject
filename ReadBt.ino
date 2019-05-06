#include <SoftwareSerial.h>

// defines pins numbers
const int myRx = 10;
const int myTx = 11;
const int stepPin = 3; 
const int dirPin = 4; 

// Constants
const int StepsPerRev = 200;
const int Pitch = 2; // (linear movement per rotation) mm
const int StepsPerMM = StepsPerRev / Pitch; // 100 steps
const int distanceBetweenContainers = 100; // mm

// Variables
String str = "";
char c = 0;
int currentContainer = 1;

SoftwareSerial MySerial(myRx, myTx);

void controlStepper(int dir, int numOfSteps)
{
  digitalWrite(dirPin, dir); // Enables the motor to move in a particular direction
  // Makes 200 pulses for making one full cycle rotation
  for(int x = 0; x < numOfSteps; x++)
  {
    digitalWrite(stepPin,HIGH); 
    delayMicroseconds(500); 
    digitalWrite(stepPin,LOW); 
    delayMicroseconds(500);
  }
}

int CalcNumSteps(int numContainer)
{
  int container = numContainer - currentContainer;
  if (container < 0)
    container = container * -1;
  return (container * distanceBetweenContainers) * StepsPerMM;
}

void setup() {
  Serial.begin(115200); // Console
  MySerial.begin(115200); // HC-06

  // Sets the two pins as Outputs
  pinMode(stepPin,OUTPUT); 
  pinMode(dirPin,OUTPUT);

  Serial.write("Beginning\n");
}

void loop() {
  // put your main code here, to run repeatedly:
  while (MySerial.available())
  {
    c = (char)MySerial.read();
    str += c;
    
    if (c == ';')
    {
      char dir = str[0];
      char container = str[1];

      if (dir == 'f')
        controlStepper(HIGH, CalcNumSteps(container - 'a'));
      else
        controlStepper(LOW, CalcNumSteps(container - 'a'));

      c = 0;
      str = "";

      // Wait for command to continue
      delay(1000);
    }

    if (c == '$')
    {
      controlStepper(LOW, CalcNumSteps(0));

      delay(1000);
    }
  }
}
