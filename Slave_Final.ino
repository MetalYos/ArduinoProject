// Include the required Wire library for I2C
#include <Wire.h>
#include <Servo.h>

Servo container1Servo;
Servo container2Servo;
Servo container4Servo;
Servo container5Servo;
Servo container6Servo;

Servo bigHitterServo;
Servo smallHitterServo;

const int servo1Pin = 2;
const int servo2Pin = 3;
const int servo3Pin = 4;
const int servo4Pin = 5;
const int servo5Pin = 6;
const int bigHitterPin = 7;
const int smallHitterPin = 8;
const int liquidRelayWater = 9; 
const int liquidRelayOil = 10;

const int container1Open = 180;
const int container1Close = 70;
const int container2Open = 160;
const int container2Close = 70;
const int container4Open = 40;
const int container4Close = 110;
const int container5Open = 90;
const int container5Close = 30;
const int container6Open = 160;
const int container6Close = 80;

bool openContainer = false;
char container;

String str = "";

void openSmallContainer(int container, int openAngle, int closeAngle, int numOfTimes)
{
  str = "";
  
  Serial.println("small container");
  for (int i = 0; i < numOfTimes; i++)
  {
    //hitters(10);

    if (openAngle >= closeAngle)
    {
      // Open Servo
      for (int pos = closeAngle; pos <= openAngle; pos += 1)
      {
        if (container == 4)
          container4Servo.write(pos);
        else if (container == 5)
          container5Servo.write(pos);
        else
          container6Servo.write(pos);
        delay(5);
      }
      delay(4000);
    
      // Close Servo
      for (int pos = openAngle; pos >= closeAngle ; pos -=1)
      {
        if (container == 4)
          container4Servo.write(pos);
        else if (container == 5)
          container5Servo.write(pos);
        else
          container6Servo.write(pos);
        delay(5);
      }
      delay(4000);
    }
    else
    {
      // Open Servo
      for (int pos = closeAngle; pos >= openAngle; pos -= 1)
      {
        if (container == 4)
          container4Servo.write(pos);
        else if (container == 5)
          container5Servo.write(pos);
        else
          container6Servo.write(pos);
        delay(5);
      }
      delay(4000);
    
      // Close Servo
      for (int pos = openAngle; pos <= closeAngle ; pos +=1)
      {
        if (container == 4)
          container4Servo.write(pos);
        else if (container == 5)
          container5Servo.write(pos);
        else
          container6Servo.write(pos);
        delay(5);
      }
      delay(4000);
    }

    //hitters(10);    
  }
}

void OpenLiquidContainer(int container, int numOfTimes)
{
  str = "";
  
  for (int i = 0; i < numOfTimes; i++)
  {
    if (container == 7) // water
    {
      // Open container
      digitalWrite(liquidRelayWater, LOW);
      delay(1000);
      // Close container
      digitalWrite(liquidRelayWater, HIGH);
      delay(1000);
    }
    else
    {
      // Open container
      digitalWrite(liquidRelayOil, LOW);
      delay(1500);
      // Close container
      digitalWrite(liquidRelayOil, HIGH);
      delay(1000);
    }
  }
}

void openBigContainer(int container, int openAngle, int closeAngle, int delayTime)
{
  str = "";
  int pos = 0;
  
  //hitters(10);

  if (openAngle >= closeAngle)
  {
    // Open Servo
    for (pos = closeAngle; pos <= openAngle; pos += 1) {
      if (container == 1)
        container1Servo.write(pos);
      else if (container == 2)
        container2Servo.write(pos);
      delay(5);
    }
    delay(delayTime);

    // Close Servo
    for (pos = openAngle; pos >= closeAngle; pos -= 1) {
      if (container == 1)
        container1Servo.write(pos);
      else if (container == 2)
        container2Servo.write(pos);
      delay(5);
    }
    delay(delayTime);
  }
  else
  {
    // Open Servo
    for (pos = closeAngle; pos >= openAngle; pos -= 1) {
      if (container == 1)
        container1Servo.write(pos);
      else if (container == 2)
        container2Servo.write(pos);
      delay(5);
    }
    delay(delayTime);

    // Close Servo
    for (pos = openAngle; pos <= closeAngle; pos += 1) {
      if (container == 1)
        container1Servo.write(pos);
      else if (container == 2)
        container2Servo.write(pos);
      delay(5);
    }
    delay(delayTime);
  }

  //hitters(10);
}

void setup() {
  Serial.begin(9600);
  Serial.println("Begin");
  
  // Start the I2C Bus as Slave on address 9
  Wire.begin(9); 
  // Attach a function to trigger when something is received.
  Wire.onReceive(receiveEvent);

  // Init servos
  container1Servo.attach(servo1Pin);
  container2Servo.attach(servo2Pin);
  container4Servo.attach(servo3Pin);
  container5Servo.attach(servo4Pin);
  container6Servo.attach(servo5Pin);
  bigHitterServo.attach(bigHitterPin);
  smallHitterServo.attach(smallHitterPin);

  // Init relay pins
  pinMode(liquidRelayWater, OUTPUT);
  pinMode(liquidRelayOil, OUTPUT);
  
  // Close liquid containers
  digitalWrite(liquidRelayWater, HIGH);
  digitalWrite(liquidRelayOil, HIGH);

  // Move servos to closed position
  container1Servo.write(container1Close);
  container2Servo.write(container2Close);
  container4Servo.write(container4Close);
  container5Servo.write(container5Close);
  container6Servo.write(container6Close);
}

void receiveEvent(int bytes) {
  str = "";
  while(Wire.available() > 0)
  {
    char c = Wire.read();
    str += c;

    if (c == '^')
    {
      while (Wire.available() > 0)
        Wire.read();
      
      break;
    }
  }
  //Serial.println(str);
}

void parseCommand(String str)
{
  str.trim();
  
  if (str != "")
    Serial.println(str);
  
  if (str.startsWith("1o^") || str.endsWith("1o^"))
  {
    openBigContainer(1, container1Open, container1Close, 1000);
  }
  
  if (str.startsWith("2o^") || str.endsWith("2o^"))
  {
    openBigContainer(2, container2Open, container2Close, 500);
  }

  // #container;#times^ - for example: 4;004^
  if (str.startsWith("4;"))
  {
    String numTimesStr = "";
    for (int i = 2; i < 5; i++)
      numTimesStr += str[i];
    int numTimes = numTimesStr.toInt();
    openSmallContainer(4, container4Open, container4Close, numTimes);
  }
  if (str.startsWith("5;"))
  {
    String numTimesStr = "";
    for (int i = 2; i < 5; i++)
      numTimesStr += str[i];
    int numTimes = numTimesStr.toInt();
    openSmallContainer(5, container5Open, container5Close, numTimes);
  }
  if (str.startsWith("6;"))
  {
    String numTimesStr = "";
    for (int i = 2; i < 5; i++)
      numTimesStr += str[i];
    int numTimes = numTimesStr.toInt();
    openSmallContainer(6, container6Open, container6Close, numTimes);
  }
  if (str.startsWith("7;"))
  {
    String numTimesStr = "";
    for (int i = 2; i < 5; i++)
      numTimesStr += str[i];
    int numTimes = numTimesStr.toInt();
    OpenLiquidContainer(7, numTimes);
  }
  if (str.startsWith("8;"))
  {
    String numTimesStr = "";
    for (int i = 2; i < 5; i++)
      numTimesStr += str[i];
    int numTimes = numTimesStr.toInt();
    OpenLiquidContainer(8, numTimes);
  }
}

void hitters(int numTimes)
{
  int pos = 0;
  for (int i = 0; i < numTimes; i++)
  {
    // Big hitter open
    for (pos = 70; pos <= 130; pos += 1) {
      bigHitterServo.write(pos);
      delay(5);
    }
    // Small hitter open
    for (pos = 70; pos <= 110; pos += 1) {
      smallHitterServo.write(pos);
      delay(5);
    }
    // Big hitter close
    delay(3);
    for (pos = 130; pos >= 70 ; pos -=1) {
      bigHitterServo.write(pos);
      delay(3);
    }
    delay(3);
    // Small hitter cose
    for (pos = 110; pos >= 70 ; pos -=1) {
      smallHitterServo.write(pos);
      delay(3);
    }
    delay(5);
  }
}

void loop() {
  parseCommand(str);
}
