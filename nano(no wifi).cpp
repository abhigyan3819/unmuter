#include <Wire.h>
#include <MPU6050.h>

//flex
const int flex1 = A0;
const int flex2 = A1;
const int flex3 = A2;
const int flex4 = A3;
const int flex5 = A6;


//mpu
MPU6050 mpu;

const int MIN_RANGE = -500;
const int MAX_RANGE = 500;

bool send = false;
const int count = 1;
int current = 0;
int values[6];  
int preValues[6] = {0,0,0,0,0,0};
const int theshold = 35;


void setup() {
  Serial.begin(115200);
  Wire.begin();
  mpu.initialize();

  if (mpu.testConnection()) {
    Serial.println("✅ MPU6050 connection successful");
  } else {
    Serial.println("❌ MPU6050 connection failed");
  }

  //flex pinMode
  pinMode(flex1,INPUT);
  pinMode(flex2,INPUT);
  pinMode(flex3,INPUT);
  pinMode(flex4,INPUT);
  pinMode(flex5,INPUT);
}

void loop() {

  const int flexData1 = analogRead(flex1);
  const int flexData2 = analogRead(flex2);
  const int flexData3 = analogRead(flex3);
  const int flexData4 = analogRead(flex4);
  const int flexData5 = analogRead(flex5);
  

  int16_t ax, ay, az;
  int16_t gx, gy, gz;

  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  int scaledAX = constrain(map(ax, -17000, 17000, -500, 500), -500, 500);
  int scaledAY = constrain(map(ay, -17000, 17000, -500, 500), -500, 500);
  int scaledAZ = constrain(map(az, -17000, 17000, -500, 500), -500, 500);
  int scaledGX = constrain(map(gx, -25000, 25000, -500, 500), -500, 500);
  int scaledGY = constrain(map(gy, -25000, 25000, -500, 500), -500, 500);
  int scaledGZ = constrain(map(gz, -25000, 25000, -500, 500), -500, 500);

  values[0] = scaledAX;
  values[1] = scaledAY;
  values[2] = scaledAZ;
  values[3] = scaledGX;
  values[4] = scaledGY;
  values[5] = scaledGZ;

  bool stable = true;
  for (int i = 0; i < 6; i++) {
    int d = abs(values[i] - preValues[i]);
    if (d > theshold) {        
      stable = false;          
      current = 0;             
      break;                   
    }
  }

  if(stable){
    stable = false;
    if(!send){
      Serial.print("SENDIND");
      send = true;
      const int DATA[11] = {scaledAX,scaledAY,scaledAZ,scaledGX,scaledGY,scaledGZ,flexData1,flexData2,flexData3,flexData4,flexData5};
      Serial.print("[");
      for (int i = 0; i < 11; i++) {
        Serial.print(DATA[i]);
        if (i < 10) Serial.print(", ");
      }
      Serial.println("]");
    }
    Serial.print("✅STABLE");
  }else{
    send = false;
  }

  preValues[0] = scaledAX;
  preValues[1] = scaledAY;
  preValues[2] = scaledAZ;
  preValues[3] = scaledGX;
  preValues[4] = scaledGY;
  preValues[5] = scaledGZ;

  Serial.print("[");
  for (int i = 0; i < 6; i++) {
    Serial.print(values[i]);
    if (i < 5) Serial.print(", ");
  }
  Serial.println("]");

  delay(300);
}
