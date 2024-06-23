#define BLYNK_TEMPLATE_ID "TMPL6imYcIqET"
#define BLYNK_TEMPLATE_NAME "DHT11"
#define BLYNK_AUTH_TOKEN "mgvEuHXRRVIVqNxS3ReB-PBb3L3jwjFR"
#define FIREBASE_HOST "robotcar-6baee-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH_TOKEN "e6mMbIyjkch8EAnCQm8eLyQmZVpQChi4PVeqor7d"
#include <FirebaseESP32.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>
#include "WiFi.h"
#include "WebServer.h"
#include "DHT11.h"
#include <HardwareSerial.h>
const char* ssid = "Cong Huong";
const char* password = "11092023";
// Thư viện điều khiển servo
 
// Khai báo đối tượng myservo dùng để điều khiển servo
Servo myservo;          
       // Khai báo chân analog đọc biến trở điều khiển servo
int servoPin = 26;       // Khai báo chân điều khiển servo
int dhtPin = 15;
int motorPin1 = 13; 
int motorPin2 = 12; 
int motorPin3 = 14; 
int motorPin4 = 27; 
// int enablePin = 25; 
int trigPin = 25;
int echoPin = 33;
String data ="";
bool autoMode = false;
#define PWM_CHANNEL 0  // Kênh PWM
#define PWM_FREQUENCY 5000  // Tần số PWM 5kHz
#define PWM_RESOLUTION 8  // Độ phân giải 8 bit (0-255)
//define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701
HardwareSerial SerialEsp(1);
DHT11 dht(dhtPin);
WebServer server(80);
FirebaseData firebaseData;
FirebaseJson json;
FirebaseConfig config;
FirebaseAuth auth;
float getDistance()
{

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(4);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  long duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance
  float distanceCm = duration * SOUND_SPEED/2;
  
  // Convert to inches
  float distanceInch = distanceCm * CM_TO_INCH;
  
  // Prints the distance in the Serial Monitor
  // Serial.print("Distance (cm): ");
  // Serial.println(distanceCm);
  return (distanceCm);
}
// int collisionChecking()
// {
//   int check = 0;
//   float distance;
//   for (int degree = 45; degree <=135; degree += 5){
//     myservo.write(degree);
//     distance = getDistance();
//     if (distance < 30) {
//       check = 1;
//     }
//     delay(10);
//   }
//   for (int degree = 135; degree >=45; degree -= 5){
//     myservo.write(degree);
//     distance = getDistance();
//     if (distance < 30) {
//       check = 1;
//     }
//     delay(10);
//   }
//   return check;
// }

void movingForward()
{
  if (getDistance()<20){
    Serial.println("Can't move. Collision Detected.");
  }
  else{
    Serial.println("Moving Forward");
    digitalWrite(motorPin2, LOW);
    digitalWrite(motorPin1, HIGH); 
    digitalWrite(motorPin3, LOW);
    digitalWrite(motorPin4, HIGH); 
  }
}
void movingBackward()
{
  Serial.println("Moving Backwards");
  digitalWrite(motorPin2, HIGH);
  digitalWrite(motorPin1, LOW); 
  digitalWrite(motorPin3, HIGH);
  digitalWrite(motorPin4, LOW); 
}

void movingLeft() {
  Serial.println("Turning Left");
  digitalWrite(motorPin2, HIGH);
  digitalWrite(motorPin1, LOW); 
  digitalWrite(motorPin3, LOW);
  digitalWrite(motorPin4, HIGH); 
}

void movingRight() {
  Serial.println("Turning Right");
  digitalWrite(motorPin2, LOW);
  digitalWrite(motorPin1, HIGH); 
  digitalWrite(motorPin3, HIGH);
  digitalWrite(motorPin4, LOW); 
}

void stopMoving()
{
  Serial.println("Motor stopped");
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, LOW);
  digitalWrite(motorPin3, LOW);
  digitalWrite(motorPin4, LOW); 
}
bool notBrake = false;
bool isForward = false;
void handleForward(){
  server.send(200,"text/plain","Moving Forward");
  isForward = true;
  notBrake = true;
}

void handleMovingForward(){
  if (isForward && getDistance()>20) {
    movingForward();
  }
  if (getDistance()<20 && notBrake) {
    Serial.println("Collision Detected.");
    stopMoving();
    notBrake = false;
  }
}
void handleBackward(){
  server.send(200,"text/plain","Moving Backward");
  movingBackward();
}

void handleLeft(){
  server.send(200,"text/plain","Turning Left");
  movingLeft();
}

void handleRight(){
  server.send(200,"text/plain","Turning Right");
  movingRight();
}

void handleStop(){
  server.send(200,"text/plain","Stop Moving");
  isForward = false;
  stopMoving();

}

int degree = 90;
bool isStop = true;
bool isLeft = false;
bool isRight = false;

void handleServoLeft(){
  server.send(200,"text/plain","Servo Turning Left");
  Serial.println("Servo Turning Left");
  isStop = false;
  isLeft = true;
}

void handleServoRight(){
  server.send(200,"text/plain","Servo Turning Right");
  Serial.println("Servo Turning Right");
  isStop = false;
  isRight = true;
}

void handleServoStop(){
  server.send(200,"text/plain","Servo Stop Turning");
  Serial.println("Servo Stop Turning");
  isStop = true;
  isRight = false;
  isLeft = false;
}

void handleServo(){
  if (!isStop){
    if (isLeft && degree<180) {
      myservo.write(++degree);
      delay(10);
    }
    if (isRight && degree>0) {
      myservo.write(--degree);
      delay(10);
    }
  }
}

int humid,temp;

void handleDHT(){
  dht.readTemperatureHumidity(temp, humid);
  String str;
  str = "Temp: " + String(temp) + " ;Humid: " + String(humid); 
  server.send(200,"text/plain",str);
  Firebase.setFloat(firebaseData, "/ESP32_APP/TEMPERATURE", temp);
  Firebase.setFloat(firebaseData, "/ESP32_APP/HUMIDITY", humid);
  Serial.println(str);
}

int trans = 4;
int recei = 2;
void setup ()
{
  // Cài đặt chức năng điều khiển servo cho chân servoPin
  Serial.begin(9600); // Starts the serial communication
  SerialEsp.begin(9600,SERIAL_8N1,trans,recei);
  pinMode(motorPin1,OUTPUT);
  pinMode(motorPin2,OUTPUT);
  pinMode(motorPin3,OUTPUT);
  pinMode(motorPin4,OUTPUT);
  stopMoving();
  pinMode(trigPin,OUTPUT);
  pinMode(echoPin,INPUT);
  // pinMode(enablePin,OUTPUT);
  // analogWrite(enablePin,150);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Waiting for ESP32 CAM to connect to Wifi");
  int count = 0;
  while (!SerialEsp.available() && count<=5000){
    delay(250);
    Serial.print(".");
    count = count + 250;
  } 
  if (count > 5000) {
    Serial.println("Failed to receive IP.");
  }
  else{
    String msg = SerialEsp.readString();
    Serial.println("ESP32 CAM connected to Wifi");
    Serial.print("IP Address: ");
    Serial.println(msg);
  }
  Blynk.begin(BLYNK_AUTH_TOKEN,ssid,password);
  Serial.print("Connecting to Blynk");
  while (Blynk.connected() == false) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to Blynk");
  isStop = true;
  isRight = false;
  isLeft = false;
  degree = 90;
  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH_TOKEN;
  Firebase.begin(&config,&auth);
  myservo.attach(servoPin); 
  myservo.write(degree);
  server.on("/forward",handleForward);
  server.on("/backward",handleBackward);
  server.on("/left",handleLeft);
  server.on("/right",handleRight);
  server.on("/stop",handleStop);
  server.on("/rightCam",handleServoRight);
  server.on("/leftCam",handleServoLeft);
  server.on("/stopCam",handleServoStop);
  server.on("/getDHT",handleDHT);
  server.begin();

}
void loop ()
{ 
  // dht.readTemperatureHumidity(temp, humid);
  Blynk.virtualWrite(V2,WiFi.localIP().toString());
  // Blynk.virtualWrite(V1,temp);
  // Blynk.virtualWrite(V0,humid);
  Blynk.run();
  server.handleClient();
  handleMovingForward();
  handleServo();
  
}
