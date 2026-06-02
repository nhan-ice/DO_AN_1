from flask import Flask, render_template, jsonify
import requests
import threading 

app = Flask(__name__)

MOTOR_ESP_IP = "http://172.20.10.2" 

status_log = "Hệ thống đang chờ xe..."
is_waiting_for_ai = False # Biến trạng thái: Có đang chờ AI xác nhận không?
ai_timer = None 

def reset_status():
    global status_log
    status_log = "Hệ thống đang chờ xe..."
    print("\n>>> [SERVER] Đã reset trạng thái web về chế độ chờ.")

def handle_stranger_alert():
    global status_log, is_waiting_for_ai
    is_waiting_for_ai = False
    status_log = "❌ CẢNH BÁO: Quá 7s không nhận ra xe. ĐÍCH THỊ LÀ XE LẠ!"
    print("\n>>> [HỆ THỐNG] Đã khóa cổng. Cảnh báo xe lạ!")
    try:
        requests.get(f"{MOTOR_ESP_IP}/alarm", timeout=3) 
    except Exception as e:
        print("Không thể kích hoạt còi báo động trên Motor!")
    threading.Timer(8.0, reset_status).start()

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/get_status')
def get_status():
    global status_log
    return jsonify({"status": status_log})

@app.route('/vehicle_approaching', methods=['GET'])
def vehicle_approaching():
    global status_log, is_waiting_for_ai, ai_timer
    
    if not is_waiting_for_ai: 
        print("\n>>> 1. [SIÊU ÂM] Có xe cách cổng dưới 15cm!")
        status_log = "🔍 Có xe trước cổng! Đang chờ Camera AI phân tích..."
        is_waiting_for_ai = True
        
        if ai_timer is not None:
            ai_timer.cancel()
        ai_timer = threading.Timer(7.0, handle_stranger_alert)
        ai_timer.start()
        
    return "OK", 200

@app.route('/ai_detect', methods=['GET'])
def ai_detect():
    global status_log, is_waiting_for_ai, ai_timer
    
    print("\n>>> 2. [CAMERA] AI vừa soi thấy xe chủ nhà!")
    
    if is_waiting_for_ai:
        print(">>> [HỆ THỐNG] Khớp dữ liệu! Mở cổng ngay!")
        ai_timer.cancel() 
        is_waiting_for_ai = False
        
        status_log = "✅ Đã xác nhận xe chủ nhà. Cửa mở, chờ bấm nút đóng..."
        
        try:
            requests.get(f"{MOTOR_ESP_IP}/open", timeout=3)
        except:
            print("Lỗi kết nối tới mạch Motor!")
            
        threading.Timer(15.0, reset_status).start()
        return "Opened", 200
    else:
        print(">>> [HỆ THỐNG] Camera thấy xe nhưng HC-SR04 không báo có người ở cổng -> BỎ QUA.")
        return "Ignored", 200

@app.route('/exit_update', methods=['GET'])
def exit_update():
    global status_log
    print("\n>>> [NÚT BẤM] Chủ nhân bấm nút mở cửa từ bên trong!")
    status_log = "🚗 Đang mở cổng cho xe xuất bến..."
    
    threading.Timer(15.0, reset_status).start() 
    return "OK", 200

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)