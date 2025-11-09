#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  mpu.initialize();
}

void loop() {
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

  delay(150);
}
