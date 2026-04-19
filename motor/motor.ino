#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

const char* ssid = "PhuocHanh";
const char* password = "tuyetnhung";

const int IN3 = 5;  
const int IN4 = 18; 
const int IR_PIN = 4; 

const char* pythonServerURL = "http://192.168.1.7:5000/exit_update"; 
unsigned long lastExitTime = 0; // bien nhieu hong ngoai

WebServer server(80);


int doorState = 0; 
unsigned long stateStartTime = 0; 

void startDoorCycle() {
  if (doorState == 0) { 
    Serial.println("\n>>> KÍCH HOẠT CHU TRÌNH CỬA <<<");
    doorState = 1; 
    stateStartTime = millis(); 
    
    Serial.println("1. Dang keo cua len (5s)...");
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  }
}

void handleDoorStates() {
  if (doorState == 0) return; 
  unsigned long currentMillis = millis(); 

  if (doorState == 1 && (currentMillis - stateStartTime >= 5000)) {
    doorState = 2; 
    stateStartTime = currentMillis; 
    Serial.println("2. Dung han cho xe di qua (5s)...");
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
  }
  
  else if (doorState == 2 && (currentMillis - stateStartTime >= 5000)) {
    doorState = 3; 
    stateStartTime = currentMillis; 
    Serial.println("3. Dang ha cua xuong (5s)...");
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  }
  
  else if (doorState == 3 && (currentMillis - stateStartTime >= 5000)) {
    doorState = 0; 
    Serial.println("4. Hoan thanh. Khoa cua.");
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(IR_PIN, INPUT);
  
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nIP Motor:");
  Serial.println(WiFi.localIP()); 

  server.on("/open", HTTP_GET, []() {
    server.send(200, "text/plain", "Da nhan lenh tu Python. Mo cua!");
    startDoorCycle(); 
  });

  server.begin();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Rot mang Wi-Fi! Dang ket noi lai...");
    WiFi.reconnect();
  }
  server.handleClient();

  handleDoorStates();

  if (digitalRead(IR_PIN) == LOW && (millis() - lastExitTime > 20000)) {
    Serial.println(">>> PHAT HIEN XE DI RA! <<<");

    if(WiFi.status() == WL_CONNECTED){
      HTTPClient http;
      http.begin(pythonServerURL);
      http.GET(); 
      http.end();
    }

    startDoorCycle(); 
    lastExitTime = millis(); 
  }
}