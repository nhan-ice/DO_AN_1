#include <ESP32Servo.h>

// Định nghĩa các chân GPIO trên ESP32
#define RAIN_SENSOR_PIN 4  // Chân D0 của cảm biến mưa
#define SERVO_PIN 18       // Chân tín hiệu điều khiển Servo

Servo windowServo; // Tạo đối tượng servo

// Biến lưu trạng thái rèm để tránh gửi lệnh quay servo liên tục gây giật/nóng motor
bool isCurtainClosed = false; 

void setup() {
  Serial.begin(115200); // Tốc độ baud tiêu chuẩn của ESP32
  
  // Cấu hình chân cảm biến mưa là tín hiệu đầu vào
  pinMode(RAIN_SENSOR_PIN, INPUT);

  // Khởi tạo Timer cho Servo (Bắt buộc đối với ESP32)
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  
  windowServo.setPeriodHertz(50);      // Tần số xung chuẩn cho servo (50Hz)
  windowServo.attach(SERVO_PIN, 500, 2400); // Gắn chân và thiết lập độ rộng xung
  
  // Trạng thái khởi động hệ thống Nhà thông minh: Mở rèm (Góc 0 độ)
  windowServo.write(0);
  Serial.println("He thong khoi dong. Rem dang mo.");
}

void loop() {
  // Đọc tín hiệu Digital từ cảm biến mưa
  // Có mưa: tín hiệu LOW (0) | Tạnh mưa: tín hiệu HIGH (1)
  int rainState = digitalRead(RAIN_SENSOR_PIN);

  if (rainState == LOW && !isCurtainClosed) {
    // Phát hiện có nước rớt vào cảm biến và rèm hiện tại đang mở
    Serial.println("Phat hien troi mua! Dang dong rem...");
    windowServo.write(90); // Xoay servo góc 90 độ để kéo rèm
    isCurtainClosed = true;
    
    // -> NƠI TÍCH HỢP WEB SERVER: 
    // Tại vị trí này trong Đồ án 1, bạn sẽ chèn thêm code Socket.IO 
    // để gửi bản tin cập nhật trạng thái "Rèm đã đóng" lên giao diện Web.
  } 
  else if (rainState == HIGH && isCurtainClosed) {
    // Tấm cảm biến đã khô (tạnh mưa) và rèm đang đóng
    Serial.println("Troi tanh mua. Dang mo rem...");
    windowServo.write(0); // Xoay servo về góc 0 độ để mở rèm
    isCurtainClosed = false;
    
    // -> NƠI TÍCH HỢP WEB SERVER:
    // Gửi bản tin cập nhật trạng thái "Rèm đã mở" lên giao diện Web.
  }

  // Tạm dừng 0.5s trước khi đọc lại cảm biến để hệ thống ổn định
  delay(500); 
}