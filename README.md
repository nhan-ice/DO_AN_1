#  Smart Parking Gate with ESP32-CAM & Edge AI

## Tổng quan

Hệ thống được xây dựng theo kiến trúc phân tán gồm ba thành phần chính:

- **ESP32-CAM:** thu nhận hình ảnh và chạy mô hình Edge AI trên thiết bị.
- **ESP32 Motor:** đọc cảm biến, điều khiển động cơ cửa cuốn, nút nhấn và buzzer.
- **Python Flask Server:** máy chủ trung tâm điều phối trạng thái, nhận tín hiệu từ ESP32-CAM và gửi lệnh đến ESP32 Motor.

Luồng hoạt động cơ bản:

```text
        ┌─────────────────────┐
        │   HC-SR04 / SR04T   │
        │ Phát hiện vật thể   │
        └──────────┬──────────┘
                   │
                   ▼
        ┌─────────────────────┐
        │       ESP32         │
        │ Điều khiển & logic  │
        └──────────┬──────────┘
                   │ Wi-Fi
                   ▼
        ┌─────────────────────┐
        │     ESP32-CAM       │
        │ Camera + Edge AI    │
        └──────────┬──────────┘
                   │ HTTP GET
                   ▼
        ┌─────────────────────┐
        │   Python / Flask    │
        │  Server điều phối   │
        └───────┬───────┬─────┘
                │       │
          mở cửa│       │cảnh báo
                ▼       ▼
        ┌──────────┐ ┌─────────┐
        │ L298N +  │ │ Buzzer  │
        │ DC Motor │ │         │
        └──────────┘ └─────────┘
```

### 🖼️ Kiến trúc hệ thống

<p align="center">
  <img src="assets/architecture.png" alt="Kiến trúc hệ thống nhà xe thông minh" width="760">
</p>

---

## ✨ Chức năng chính

### 1. Phát hiện phương tiện

Cảm biến siêu âm liên tục đo khoảng cách. Khi phát hiện vật thể trong vùng kích hoạt (khoảng cách dưới **15 cm** theo mô hình thử nghiệm), hệ thống kích hoạt quá trình nhận diện.

### 2. Nhận diện bằng Edge AI

ESP32-CAM:

1. Thu nhận hình ảnh từ camera.
2. Tiền xử lý ảnh.
3. Chạy mô hình học máy Edge Impulse ngay trên thiết bị.
4. Lọc kết quả theo ngưỡng confidence **> 85%**.
5. Gửi tín hiệu xác nhận về server qua Wi-Fi.

Mô hình được huấn luyện để phân biệt:

- `XE XANH`
- `XE VANG`
- `BACKGROUND`

### 3. Điều khiển cửa cuốn

Khi phương tiện hợp lệ được xác nhận:

```text
Xe đến
  ↓
HC-SR04 phát hiện
  ↓
ESP32-CAM nhận diện
  ↓
Xe hợp lệ?
 ┌───────────────┐
 │ Có            │ Không
 ▼               ▼
Mở cửa           Chờ timeout
 │               │
 ▼               ▼
Cho xe đi        Buzzer cảnh báo
```

Động cơ DC giảm tốc được điều khiển thông qua **L298N**, hỗ trợ đảo chiều và điều chỉnh tốc độ bằng PWM.

### 4. Cảnh báo xe lạ

Nếu phương tiện không thuộc dữ liệu nhận diện hoặc không nhận diện thành công trong thời gian quy định, server kích hoạt buzzer.

Trong kịch bản thử nghiệm, xe lạ dừng trước cổng quá **7 giây** sẽ kích hoạt cảnh báo.

### 5. Điều khiển thủ công

Nút nhấn vật lý cho phép chủ động điều khiển cửa, phục vụ trường hợp xe đi ra hoặc đóng cửa khẩn cấp.

### 6. Chống gửi lệnh liên tục

ESP32-CAM sử dụng cơ chế cooldown. Sau khi gửi lệnh mở cửa thành công, camera nghỉ khoảng **10 giây** để tránh gửi lặp lệnh khi xe vẫn còn trong khung hình.

---

## Phần cứng


<p align="center">
  <img src="assets/esp32-cam.jpg" alt="ESP32-CAM" width="260">
  &nbsp;&nbsp;
  <img src="assets/dc-gear-motor.jpg" alt="Động cơ DC giảm tốc" width="260">
  &nbsp;&nbsp;
  <img src="assets/l298n.jpg" alt="Module L298N" width="260">
</p>

<p align="center">
  <img src="assets/hc-sr04.jpg" alt="Cảm biến siêu âm HC-SR04" width="300">
</p>

| Thành phần | Vai trò |
|---|---|
| ESP32 | Bộ điều khiển trung tâm |
| ESP32-CAM | Camera và xử lý Edge AI |
| OV3660 | Cảm biến hình ảnh |
| HC-SR04 / HC-SR04T | Phát hiện vật thể |
| L298N | Driver điều khiển động cơ |
| DC Gear Motor | Cơ cấu kéo/mở cửa |
| Buzzer | Cảnh báo xe không hợp lệ |
| Push Button | Điều khiển thủ công |
| Power Bank | Cấp nguồn cho ESP32/ESP32-CAM |
| Nguồn pin 12V | Cấp nguồn cho động cơ/L298N |

### Nguồn cấp

Hệ thống sử dụng nguồn tách biệt:

- ESP32 và ESP32-CAM: khoảng **5V/2A hoặc 3A**.
- Động cơ DC: nguồn riêng **12V** cấp qua L298N.

Việc tách nguồn giúp hạn chế nhiễu do động cơ gây ra đối với mạch điều khiển.

---

## 🧠 Edge AI & Edge Impulse

Mô hình AI được phát triển bằng **Edge Impulse** và triển khai trực tiếp trên ESP32-CAM.

### Quy trình xây dựng mô hình

```text
Thu thập dữ liệu
       ↓
Gán nhãn Bounding Box
       ↓
Thiết kế Impulse
       ↓
Tiền xử lý ảnh
       ↓
Huấn luyện mô hình
       ↓
Đánh giá
       ↓
Quantization
       ↓
Biên dịch / triển khai trên ESP32-CAM
```

Ảnh đầu vào được cấu hình ở kích thước:

```text
96 × 96 pixels
RGB
```

Dữ liệu được thu thập bằng chính camera ESP32-CAM nhằm giúp mô hình phù hợp hơn với góc nhìn và đặc tính hình ảnh thực tế của hệ thống.

<p align="center">
  <img src="assets/training-dataset.jpg" alt="Dữ liệu huấn luyện trong Edge Impulse" width="760">
</p>

Thuật toán phát hiện đối tượng sử dụng **FOMO (Faster Objects, More Objects)**, phù hợp với các thiết bị có tài nguyên hạn chế.

---

## 📊 Kết quả thực nghiệm

Theo kết quả trong báo cáo:

| Chỉ số | Kết quả |
|---|---:|
| F1 Score | **100.0%** |
| Precision | **1.00** |
| Recall | **1.00** |
| Nhãn kiểm thử | `BACKGROUND`, `XE VANG`, `XE XANH` |
| Peak RAM | **137.7 KB** |
| Flash | **81.2 KB** |
| Inferencing time | **~1090 ms / frame** |
| Confidence threshold | **> 85%** |

Trên tập validation, mô hình phân loại chính xác cả ba lớp và không ghi nhận False Positive.

<p align="center">
  <img src="assets/model-metrics.png" alt="Kết quả đánh giá mô hình Edge Impulse" width="760">
</p>

Trong thử nghiệm thực tế, ESP32-CAM mất khoảng hơn 1 giây để chạy inference và gửi tín hiệu xác nhận. Server sau đó điều khiển ESP32 Motor để mở cửa.

---

## Mô hình thực tế

<p align="center">
  <img src="assets/prototype-front.jpg" alt="Mô hình nhà xe - góc nhìn phía trước" width="420">
  &nbsp;&nbsp;
  <img src="assets/prototype-inside.jpg" alt="Mô hình nhà xe - bố trí phần cứng bên trong" width="420">
</p>

Mô hình thử nghiệm trong báo cáo gồm phần cửa, cảm biến, camera, mạch điều khiển, driver động cơ và cơ cấu truyền động.

### Giao diện và kết quả kiểm thử

<p align="center">
  <img src="assets/web-dashboard.jpg" alt="Giao diện web giám sát" width="760">
</p>

<p align="center">
  <img src="assets/test-recognized.jpg" alt="Kết quả nhận diện phương tiện" width="300">
  &nbsp;&nbsp;
  <img src="assets/test-ai.jpg" alt="Kết quả phân tích AI" width="300">
  &nbsp;&nbsp;
  <img src="assets/test-warning.jpg" alt="Cảnh báo xe lạ" width="300">
</p>

---

## Kiến trúc phần mềm

### Python / Flask Server

Server đóng vai trò trung tâm điều phối:

- Nhận tín hiệu từ ESP32-CAM.
- Quản lý trạng thái hệ thống.
- Quản lý bộ đếm thời gian an ninh.
- Quyết định mở cửa hoặc kích hoạt buzzer.
- Gửi lệnh HTTP đến ESP32 Motor.
- Cung cấp giao diện Web giám sát.
- Xử lý trạng thái khi mất kết nối.

### ESP32 Motor

ESP32 Motor hoạt động theo mô hình FSM với các trạng thái chính:

```text
0 - Cửa đang đóng / chờ
1 - Đang mở cửa
2 - Cửa mở hoàn toàn / chờ xe
3 - Đang đóng cửa
```

Chương trình không sử dụng `delay()` dài để giữ hệ thống đứng yên mà dựa trên mốc thời gian, cho phép ESP32 đồng thời:

- Điều khiển PWM cho động cơ.
- Đọc cảm biến.
- Xử lý nút nhấn.
- Duy trì kết nối Wi-Fi.
- Nhận lệnh từ server.

### ESP32-CAM

ESP32-CAM hoạt động như một AI edge node:

```text
Camera
  ↓
Frame Buffer
  ↓
Edge Impulse Inference
  ↓
Confidence Filter (>85%)
  ↓
Cooldown
  ↓
HTTP GET
  ↓
Flask Server
```

Chỉ tín hiệu trigger nhẹ được gửi về server thay vì truyền toàn bộ hình ảnh qua mạng, giúp giảm tải mạng và độ trễ.

---

## Giao tiếp mạng

Các node giao tiếp với nhau qua Wi-Fi nội bộ.

ESP32-CAM đóng vai trò HTTP Client và gửi HTTP GET đến địa chỉ IP của Flask Server.

ESP32 Motor đồng thời:

- Gửi trạng thái lên server.
- Chạy Web Server nội bộ.
- Lắng nghe lệnh điều khiển trên **port 80**.

Kiến trúc tổng thể:

```text
ESP32-CAM ──────HTTP GET──────► Flask Server
                                   │
                                   │ HTTP
                                   ▼
                              ESP32 Motor
                                   │
                             ┌─────┴─────┐
                             ▼           ▼
                           L298N       Buzzer
                             │
                             ▼
                         DC Motor
```

---

## Cài đặt và triển khai

### Yêu cầu

- ESP32 / ESP32-CAM
- Camera OV3660
- HC-SR04 / HC-SR04T
- L298N
- DC Gear Motor
- Buzzer
- Push Button
- Máy tính chạy Python
- Mạng Wi-Fi nội bộ
- Edge Impulse

### ESP32-CAM

1. Kết nối ESP32-CAM với máy tính.
2. Cấu hình Wi-Fi.
3. Tích hợp thư viện/model được export từ Edge Impulse.
4. Build và upload firmware.
5. Kiểm tra camera.
6. Kiểm tra kết nối HTTP đến Flask Server.

### ESP32 Motor

1. Cấu hình Wi-Fi.
2. Kết nối HC-SR04/HC-SR04T.
3. Kết nối L298N với động cơ DC.
4. Kết nối buzzer và nút nhấn.
5. Upload firmware.
6. Kiểm tra Web Server nội bộ và khả năng nhận lệnh từ Flask Server.

---

## Các trường hợp kiểm thử

### Test 1 — Cảm biến & cơ khí

- Đưa vật thể đến dưới 15 cm.
- Kiểm tra cảm biến kích hoạt.
- Kiểm tra động cơ mở/đóng.
- Kiểm tra cửa không bị kẹt.

### Test 2 — Kết nối mạng

Kiểm tra:

- ESP32 kết nối Wi-Fi.
- ESP32-CAM kết nối Wi-Fi.
- Flask Server nhận HTTP GET.
- Server phản hồi và gửi lệnh đến ESP32 Motor.

Yêu cầu thời gian phản hồi mục tiêu: **không quá 2 giây**.

### Test 3 — Xe hợp lệ

```text
Xe hợp lệ
   ↓
AI nhận diện >85%
   ↓
Server xác nhận
   ↓
ESP32 Motor nhận lệnh
   ↓
Cửa mở
```

### Test 4 — Xe lạ

```text
Xe lạ
  ↓
Không thuộc lớp nhận diện hợp lệ
  ↓
Chờ timeout
  ↓
Buzzer cảnh báo
```

### Test 5 — Xe đi ra

Nút nhấn vật lý được sử dụng để mở cửa từ phía bên trong. Sau khi xe đi qua, hệ thống thực hiện chu trình đóng cửa.

---

## Hạn chế

Hệ thống hiện tại vẫn còn một số hạn chế:

- Hiệu suất nhận diện giảm trong điều kiện ánh sáng yếu hoặc ban đêm.
- Mô hình mới được kiểm thử ở quy mô mô hình thu nhỏ.
- Chưa được kiểm chứng với cửa cuốn thực tế có kích thước và tải trọng lớn.
- Một số trường hợp toàn bộ chu trình phản hồi chưa đạt yêu cầu thời gian thực.
- Chưa tích hợp đầy đủ cảm biến va chạm/chống kẹt.
- Chưa có camera hồng ngoại ban đêm.
- Chưa có cơ chế xử lý đầy đủ khi mất điện.
- Mô hình AI có nguy cơ overfitting do dữ liệu ban đầu được thu thập trong môi trường thí nghiệm.

---

## 🔮 Hướng phát triển

Các hướng phát triển được đề xuất:

- Thu thập thêm dữ liệu ở nhiều điều kiện ánh sáng và góc nhìn.
- Bổ sung camera hồng ngoại hoặc cảm biến PIR.
- Phát triển ứng dụng mobile để giám sát và điều khiển từ xa.
- Mở rộng từ mô hình thu nhỏ lên cửa cuốn thực tế.
- Sử dụng động cơ mạnh hơn và cơ cấu truyền động chắc chắn hơn.
- Kết nối Firebase hoặc MQTT để lưu trữ lịch sử và quản lý nhiều cửa.
- Bổ sung nhận diện biển số xe.
- Phát triển tính năng tự động tính phí.
- Bổ sung hệ thống báo động chống trộm.

