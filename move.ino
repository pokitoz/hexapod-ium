
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
#define C_SERVO_MIN  100
#define C_SERVO_MAX  475
#define SERVO_FREQ 50

void setup() {
  Serial.begin(9600);
  pwm.begin();
  pwm.setPWMFreq(SERVO_FREQ); 
  delay(10);
}

int servoLeg [] = {1, 4, 5, 7, 15, 11};
int servoMax [] = {C_SERVO_MAX, C_SERVO_MAX + 30, C_SERVO_MAX,      C_SERVO_MAX, C_SERVO_MAX + 30, C_SERVO_MAX + 30};
int servoMin [] = {C_SERVO_MIN - 30, C_SERVO_MIN, C_SERVO_MIN - 20, C_SERVO_MIN, C_SERVO_MIN     , C_SERVO_MIN };
int servoHalf [] = {185, 300, 200, 200, 310, 220};

int servoMid [] = {2, 3, 6, 8, 10, 12};
int servoMidVal[] = {300, 270, 270, 140, 250, 110};
int servoMidValInc[] = {220, 220, 220, 120, 280, 180};

int sizeServo = 6;
int sizeServoMid = 6;

void legUp(int id)
{
  if (id <= 2)
  {
    pwm.setPWM(servoLeg[id], 0, servoMax[id]);
  }
  else
  {
    pwm.setPWM(servoLeg[id], 0, servoMin[id]);
  }
}

void legDown(int id)
{
  if (id <= 2)
  {
    pwm.setPWM(servoLeg[id], 0, servoMin[id]);
  }
  else
  {
    pwm.setPWM(servoLeg[id], 0, servoMax[id]);
  }
}

void legHalf(int id)
{
  pwm.setPWM(servoLeg[id], 0, servoHalf[id]);
}

void legMiddle(int id)
{
  pwm.setPWM(servoMid[id], 0, servoMidVal[id]);
}

void legInc(int id)
{
  pwm.setPWM(servoMid[id], 0, servoMidValInc[id]);
}

void loop() {
    for (int i = 0; i < sizeServoMid; i++)
    {
      legInc(i);
    }

    for (int i = 0; i < sizeServo; i++) {
      if (i % 2)
      {
        legUp(i);
        legInc(i);
      }
      else
      {
        legHalf(i);
        legMiddle(i);
      }
    }

    delay(1000);
    for (int i = 0; i < sizeServo; i++) {
        legHalf(i);
    }

    delay(1000);

    for (int i = 0; i < sizeServo; i++) {

      if (i % 2)
      {
        legHalf(i);
        legMiddle(i);
      }
      else
      {
        legUp(i);
        legInc(i);
      }
    }

    delay(1000);
    for (int i = 0; i < sizeServo; i++) {
      legHalf(i);
    }

    delay(1000);
}
