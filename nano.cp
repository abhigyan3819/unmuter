#include <SoftwareSerial.h>
#include <Wire.h>
#include <MPU6050.h>

int DATA[11];

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


//wifi variable
SoftwareSerial esp(2, 3); 

String AP_ssid = "ESP32_AP";
String AP_pass = "12345678";
String serverIP = "192.168.4.1";

void sendAT(String cmd, int wait) {
  esp.println(cmd);
  delay(wait);
  while (esp.available()) {
    Serial.write(esp.read());
  }
}

void sendMessage(String msg) {
  String httpRequest = 
    "GET /send?value=" + msg + " HTTP/1.1\r\nHost: " + serverIP + "\r\nConnection: keep-alive\r\n\r\n";

  sendAT("AT+CIPSEND=" + String(httpRequest.length()), 200);
  esp.print(httpRequest);

  unsigned long start = millis();
  while (millis() - start < 1000) {
    if (esp.find("SEND OK")) break;
  }
}

void setup() {
  //wifi
  Serial.begin(9600);
  esp.begin(9600); 
  Serial.println("Reset ESP...");
  sendAT("AT+RST", 2000);

  Serial.println("Switch to STA mode...");
  sendAT("AT+CWMODE=1", 1000);

  Serial.println("Connecting to ESP32 AP...");
  sendAT("AT+CWJAP=\"" + AP_ssid + "\",\"" + AP_pass + "\"", 6000);

  Serial.println("Opening TCP connection...");
  sendAT("AT+CIPSTART=\"TCP\",\"" + serverIP + "\",80", 2000);

  Serial.println("Ready to send multiple messages!");



  //mpu
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
String arrayToString(int *arr, int len) {
  String s = "";
  for (int i = 0; i < len; i++) {
    s += String(arr[i]);
    if (i < len - 1) s += ",";
  }
  return s;
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
      break;                   
    }
  }

  if(stable){
    stable = false;
    if(!send){
      Serial.print("SENDIND");
      send = true;
      int tempData[11] = {scaledAX, scaledAY, scaledAZ, scaledGX, scaledGY, scaledGZ,
                    flexData1, flexData2, flexData3, flexData4, flexData5};
      for (int i = 0; i < 11; i++) {
        DATA[i] = tempData[i];
      }

      Serial.print("[");
      for (int i = 0; i < 11; i++) {
        Serial.print(DATA[i]);
        if (i < 10) Serial.print(", ");
      }
      Serial.println("]");
      String data = arrayToString(DATA,11);
      sendMessage(data);
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
    Serial.print(", ");
  }
  Serial.print(flexData1);Serial.print(","); 
  Serial.print(flexData2);Serial.print(","); 
  Serial.print(flexData3);Serial.print(","); 
  Serial.print(flexData4);Serial.print(","); 
  Serial.print(flexData5);
  Serial.println("]");
  
  delay(300);  
}
