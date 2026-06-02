#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

const char* ssid = "Nhan ice";
const char* password = "12345679";

const int ENB_PIN = 22; 
const int IN3 = 5;      
const int IN4 = 18;     

const int BUTTON_PIN = 4; 
const int TRIG_PIN = 12; 
const int ECHO_PIN = 13;
const int BUZZER = 14;

const char* pythonServerIP = "http://172.20.10.13:5000"; // IP CỦA LAPTOP
unsigned long lastApproachTime = 0;
unsigned long lastButtonPress = 0; 

WebServer server(80);
int doorState = 0; 
unsigned long stateStartTime = 0;

// BIẾN PHÂN BIỆT ĐI VÀO HAY ĐI RA
bool isExiting = false; 

const int pwmSpeed = 60; 

void guiThongBaoWeb(String endpoint) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.setTimeout(500); 
    http.begin(String(pythonServerIP) + endpoint);
    http.GET();
    http.end();
  }
}

int getDistance() {
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 30000); 
  if (duration == 0) return 999;
  return duration * 0.034 / 2;
}

void startDoorCycle() {
  if (doorState == 0) {
    doorState = 1; 
    stateStartTime = millis(); 
    Serial.println(">>> MOTOR: Dang mo cua DI LEN (3s)...");
    
    digitalWrite(IN3, HIGH); 
    digitalWrite(IN4, LOW);
    analogWrite(ENB_PIN, pwmSpeed);
  }
}

void handleDoorStates() {
  if (doorState == 0) return;
  unsigned long currentMillis = millis();
  
  // 1. Đang mở cửa -> Kiểm tra đủ 2.8s chưa
  if (doorState == 1 && (currentMillis - stateStartTime >= 3000)) {
    doorState = 2; 
    
    if (isExiting) {
        Serial.println(">>> MOTOR: Mo cua xong. CHO XE DI QUA CAM BIEN DE DONG...");
    } else {
        Serial.println(">>> MOTOR: Mo cua xong. CHO BAM NUT DE DONG...");
    }
    
    digitalWrite(IN3, LOW); 
    digitalWrite(IN4, LOW);
    analogWrite(ENB_PIN, 0);
  } 

  else if (doorState == 3 && (currentMillis - stateStartTime >= 3000)) {
    doorState = 0; 
    Serial.println(">>> MOTOR: Hoan thanh. Khoa cua.");
    
    digitalWrite(IN3, LOW); 
    digitalWrite(IN4, LOW);
    analogWrite(ENB_PIN, 0);
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(ENB_PIN, OUTPUT);
  pinMode(IN3, OUTPUT); 
  pinMode(IN4, OUTPUT);
  
  pinMode(BUTTON_PIN, INPUT_PULLUP); 
  pinMode(TRIG_PIN, OUTPUT); 
  pinMode(ECHO_PIN, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);
  
  digitalWrite(BUZZER, LOW);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
  }
  
  Serial.println("\nKết nối Wi-Fi thành công!");
  Serial.print("IP Motor: ");
  Serial.println(WiFi.localIP());
  
  server.on("/open", HTTP_GET, []() {
    server.send(200, "text/plain", "OK");
    isExiting = false; // Đánh dấu là đang đi VÀO
    startDoorCycle();
  });
  
  server.on("/alarm", HTTP_GET, []() {
    server.send(200, "text/plain", "Dang hu coi!");
    Serial.println(">>> MOTOR: NHAN LENH HU COI BAO DONG!");

    for(int i = 0; i < 3; i++) {
      digitalWrite(BUZZER, HIGH);
      delay(300);
      digitalWrite(BUZZER, LOW);
      delay(300);
    }
  });
  
  server.begin();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) WiFi.reconnect();
  server.handleClient();
  handleDoorStates();

  int distance = getDistance();

  if (distance > 0 && distance <= 15 && doorState == 0 && (millis() - lastApproachTime > 10000)) {
    Serial.println(">>> Sieu am thay xe! Bao cho Python...");
    guiThongBaoWeb("/vehicle_approaching");
    lastApproachTime = millis();
  }

  if (distance > 0 && distance <= 15 && doorState == 2 && isExiting == true) {
    Serial.println(">>> SIÊU ÂM: Xe dang qua cong... Chuan bi dong!");
    delay(1500); 
    
    Serial.println(">>> MOTOR: Tu dong dong cua xuong ngay!");
    doorState = 3; 
    stateStartTime = millis(); 
    
    digitalWrite(IN3, LOW); 
    digitalWrite(IN4, HIGH);
    analogWrite(ENB_PIN, pwmSpeed);
    
    isExiting = false; 
  }

  if (digitalRead(BUTTON_PIN) == LOW && (millis() - lastButtonPress > 500)) {
    lastButtonPress = millis(); 

    if (doorState == 0) {
      Serial.println(">>> NÚT BẤM: Xe di ra -> Mo cua len!");
      guiThongBaoWeb("/exit_update"); 
      isExiting = true; // Đánh dấu là đang đi RA
      startDoorCycle(); 
    } 
    else if (doorState == 2) {
      Serial.println(">>> NÚT BẤM: Ép dong cua xuong ngay!");
      doorState = 3; 
      stateStartTime = millis(); 
      
      digitalWrite(IN3, LOW); 
      digitalWrite(IN4, HIGH);
      analogWrite(ENB_PIN, pwmSpeed);
      
      isExiting = false; 
    }
  }
}