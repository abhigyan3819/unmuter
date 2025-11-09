#include <Wire.h>
#include <MPU6050.h>
#include <BluetoothSerial.h>

BluetoothSerial BT;
MPU6050 mpu;

int f1 = 4;
int f2 = 5;
int f3 = 18;
int f4 = 19;
int f5 = 23;

int f1Data;
int f2Data;
int f3Data;
int f4Data;
int f5Data;


void setup() {
  Serial.begin(9600);
  BT.begin("Unmuter");

  Wire.begin();
  mpu.initialize();

  pinMode(f1,INPUT_PULLUP);
  pinMode(f2,INPUT_PULLUP);
  pinMode(f3,INPUT_PULLUP);
  pinMode(f4,INPUT_PULLUP);
  pinMode(f5,INPUT_PULLUP);

}

void loop() {
  f1Data = digitalRead(f1);
  f2Data = digitalRead(f2);
  f3Data = digitalRead(f3);
  f4Data = digitalRead(f4);
  f5Data = digitalRead(f5);

  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  float Ax = ax / 16384.0;
  float Ay = ay / 16384.0;
  float Az = az / 16384.0;

  float pitch = atan2(Ax, sqrt(Ay*Ay + Az*Az)) * 180/PI;
  float roll  = atan2(Ay, Az) * 180/PI;

  String orientation;

  if (pitch > 35) orientation = "Forward";
  else if (pitch < -35) orientation = "Backward";
  else if (roll > 30) orientation = "Right Tilt";
  else if (roll < -30) orientation = "Left Tilt";
  else if (Az > 0.8) orientation = "Face Up";
  else if (Az < -0.8) orientation = "Face Down";
  else orientation = "Neutral";

  Serial.print("Pitch: ");
  Serial.print(pitch);
  Serial.print("  Roll: ");
  Serial.print(roll);
  Serial.print("  ->  ");
  Serial.println(orientation);

  String values = f1Data + "," + f2Data + "," + f3Data + "," + f4Data + "," + f5Data + "," + orientation;

  BT.println(values);

  delay(500);

}
