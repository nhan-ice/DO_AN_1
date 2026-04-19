from flask import Flask, render_template, jsonify
import requests
import threading 

app = Flask(__name__)

MOTOR_ESP_URL = "http://192.168.1.17/open" 

status_log = "Hệ thống đang chờ xe..."

def reset_status():
    global status_log
    status_log = "Hệ thống đang chờ xe..."
    print("\n>>> [SERVER] Đã qua chu trình 15s, reset trạng thái về chờ xe mới.")

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/get_status')
def get_status():
    global status_log
    return jsonify({"status": status_log})

@app.route('/open_door', methods=['GET'])
def trigger_door():
    global status_log
    print("\n>>> [AI CAMERA] ESP32-CAM vừa phát hiện xe của chủ nhà!")
    status_log = "Phát hiện xe! Đang gửi lệnh mở cửa..."
    
    try:
        print(f">>> [SERVER] Đang chuyển tiếp lệnh tới Motor...")
        response = requests.get(MOTOR_ESP_URL, timeout=5)
        
        if response.status_code == 200:
            print(">>> [MOTOR] Phản hồi: Động cơ đang chạy!")
            status_log = "Cửa đang mở... Chu trình 15s đang hoạt động!"
            
            threading.Timer(15.0, reset_status).start()
            
            return "OK", 200
        else:
            status_log = "Lỗi: Động cơ từ chối lệnh!"
            threading.Timer(5.0, reset_status).start() 
            return "Motor Error", 500
            
    except Exception as e:
        print(f">>> [LỖI HỆ THỐNG] Không thể kết nối Motor: {e}")
        status_log = "Lỗi: Mất kết nối mạng với mạch Motor!"
        threading.Timer(5.0, reset_status).start() 
        return "Connection Failed", 500

# --- TÍNH NĂNG MỚI: NHẬN BÁO CÁO TỪ CẢM BIẾN HỒNG NGOẠI (XE ĐI RA) ---
@app.route('/exit_update', methods=['GET'])
def exit_update():
    global status_log
    print("\n>>> [HỒNG NGOẠI] Báo cáo: Có xe đang đi ra ngoài!")
    status_log = "Phát hiện xe xuất bến... Chu trình 15s đang hoạt động!"
    
    # Kích hoạt bộ đếm 15 giây rồi tự reset trạng thái web
    threading.Timer(15.0, reset_status).start() 
    return "OK", 200

if __name__ == '__main__':
    print(">>> SERVER TRUNG TÂM ĐANG CHẠY...")
    app.run(host='0.0.0.0', port=5000)