// Include the required Wire library for I2C
#include <Wire.h>
#include <Servo.h>

Servo container1Servo;
Servo container2Servo;
Servo container3Servo;
Servo container4Servo;
Servo container5Servo;

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
    
    // Open Servo
    for (int pos = openAngle; pos <= closeAngle; pos += 1) {
      if (container == 3)
        container3Servo.write(pos);
      else if (container == 4)
        container4Servo.write(pos);
      else
        container5Servo.write(pos);
      delay(5);
    }
    delay(5000);
  
    // Close Servo
    for (int pos = closeAngle; pos >= openAngle ; pos -=1)
    {
      if (container == 3)
        container3Servo.write(pos);
      else if (container == 4)
        container4Servo.write(pos);
      else
        container5Servo.write(pos);
      delay(5);
    }
    delay(5000);

    //hitters(10);    
  }

  str = "";
}

void openBigContainer(int container, int openAngle, int closeAngle, int delayTime)
{
  int pos = 0;
  str = ""; 
  
  //hitters(10);

  if (container == 1)
  {
  // Open Servo
  for (pos = openAngle; pos <= closeAngle; pos += 1) {
    if (container == 1)
      container1Servo.write(pos);
    else if (container == 2)
      container2Servo.write(pos);
    delay(5);
  }
  delay(delayTime);

  // Close Servo
  for (pos = closeAngle; pos >= openAngle; pos -= 1) {
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
    // Close Servo
  for (pos = closeAngle; pos >= openAngle; pos -= 1) {
    if (container == 1)
      container1Servo.write(pos);
    else if (container == 2)
      container2Servo.write(pos);
    delay(5);
  }
  delay(delayTime);

  // Open Servo
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

  str = "";
}

void setup() {
  Serial.begin(9600);
  
  // Start the I2C Bus as Slave on address 9
  Wire.begin(9); 
  // Attach a function to trigger when something is received.
  Wire.onReceive(receiveEvent);

  // Init servos
  container1Servo.attach(servo1Pin);
  container2Servo.attach(servo2Pin);
  container3Servo.attach(servo3Pin);
  container4Servo.attach(servo4Pin);
  container5Servo.attach(servo5Pin);
  bigHitterServo.attach(bigHitterPin);
  smallHitterServo.attach(smallHitterPin);

  // Init relay pins
  pinMode(liquidRelayWater, OUTPUT);
  pinMode(liquidRelayOil, OUTPUT);

  digitalWrite(liquidRelayWater, LOW);
  digitalWrite(liquidRelayOil, LOW);

  // Move servos to closed position
  container2Servo.write(180);
  container4Servo.write(20);
}

void receiveEvent(int bytes) {
  if (str[str.length() - 1] == '^')
    str = "";
  for(int i=0; i< bytes ; i++)
  {
    str += (char)Wire.read();    // read one character from the I2C
  }
  Serial.println(str);
}

void parseCommand(String str)
{
  if (str.startsWith("1o^") || str.endsWith("1o^"))
  {
    openBigContainer(1, 70, 180, 1000);
    str = "";
  }
  
  if (str.startsWith("2o^") || str.endsWith("2o^"))
  {
    openBigContainer(2, 100, 180, 500);
    str = "";
  }

  // #container;#times^ - for example: 3;004^
  if (str.startsWith("3;") || str.endsWith("3;"))
  {
    String numTimesStr = "";
    for (int i = 2; i < 5; i++)
      numTimesStr += str[i];
    int numTimes = numTimesStr.toInt();
    openSmallContainer(3, 30, 100, numTimes);
    str = "";
  }
  if (str.startsWith("4;") || str.endsWith("4;"))
  {
    String numTimesStr = "";
    for (int i = 2; i < 5; i++)
      numTimesStr += str[i];
    int numTimes = numTimesStr.toInt();
    openSmallContainer(4, 20, 110, numTimes);
    str = "";
  }
  if (str.startsWith("5;") || str.endsWith("5;"))
  {
    String numTimesStr = "";
    for (int i = 2; i < 5; i++)
      numTimesStr += str[i];
    int numTimes = numTimesStr.toInt();
    openSmallContainer(5, 70, 170, numTimes);
    str = "";
  }
  if (str.startsWith("6^") || str.endsWith("6^"))
  {
    OpenLiquidContainer(6);
    str = "";
  }
  if (str.startsWith("7^") || str.endsWith("7^"))
  {
    OpenLiquidContainer(7);
    str = "";
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

void OpenLiquidContainer(int container)
{
  str = "";
  
  if (container == 6) // water
  {
    digitalWrite(liquidRelayWater, HIGH);
    delay(2000);
    digitalWrite(liquidRelayWater, LOW);
  }
  else
  {
    digitalWrite(liquidRelayOil, HIGH);
    delay(2000);
    digitalWrite(liquidRelayOil, LOW);
  }

  str = "";
}

void loop() {
  parseCommand(str);
}
