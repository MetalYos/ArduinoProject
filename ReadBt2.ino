#include <SoftwareSerial.h>

// defines pins numbers
const int myRx = 10;
const int myTx = 11;
const int stepPin = 3; 
const int dirPin = 4; 

// Constants
const int StepsPerRev = 200;
const int Pitch = 2; // (linear movement per rotation) mm
const int StepsPerMM = 100; // 100 steps
const int distanceBetweenContainers = 20; // mm

// Variables
char c = 0;
int totalSteps = 0;
String command;
bool started = false;
int prevContainer = 0;

SoftwareSerial MySerial(myRx, myTx);

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

void setup() {
  Serial.begin(115200); // Console
  MySerial.begin(115200); // HC-06

  // Sets the two pins as Outputs
  pinMode(stepPin,OUTPUT); 
  pinMode(dirPin,OUTPUT);
  Serial.write("Beginning\n");
}

int digit_to_int(char d)
{
 char str1[2];

 str1[0] = d;
 str1[1] = '\0';
 return (int) strtol(str1, NULL, 10);
}

void loop() {
  char container;
  bool valid;
  while (MySerial.available())
  {
    valid = 0;
    c = 0;
    command = "";
    while (c != ';')
    {
      c = (char)MySerial.read();
      if ( (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) 
      {
        valid = 1;
      }
      if (valid){
        command += c; 
      }  
    }  
    
    if (c == ';')
    {
      char dir = command[0];
      container = command[1];
      
      int calc = 0;
      //the difference between stations. calculated by subtracting the previous position from the current position.
      int containerDiff = 0;
      if (dir == 'f')
      {
        calc = digit_to_int(container);
        containerDiff = calc - prevContainer;
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
        prevContainer = calc; 
      }
      else
      {
        controlStepper(LOW, totalSteps);
        totalSteps = 0;
        prevContainer = 0;
      }
      
      // Wait for command to continue
      delay(3000);
      Serial.write("\nEnd Loop\n");
    }
  }
}
