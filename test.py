from flask import Flask
# Import thư viện điều khiển động cơ hoặc board mạch của bạn ở đây

app = Flask(__name__)

@app.route('/open_door', methods=['GET'])
def trigger_door():
    print(">>> NHẬN LỆNH TỪ ESP32: Đang mở cửa cuốn...")
    
    # ---------------------------------------------------------
    # VIẾT CODE ĐIỀU KHIỂN ĐỘNG CƠ CỦA BẠN VÀO ĐÂY
    # Ví dụ: Kích hoạt Relay qua Serial, hoặc gửi lệnh Socket.IO 
    # ---------------------------------------------------------
    
    return "Door Opened", 200

if __name__ == '__main__':
    print("Server Nhà Xe Thông Minh đang chạy...")
    # Đổi '0.0.0.0' để cho phép ESP32 trong mạng LAN truy cập
    app.run(host='0.0.0.0', port=5000)