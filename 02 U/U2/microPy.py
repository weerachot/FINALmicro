from machine import Pin
import socket
import network
import time

# ====== Config ======
SSID = 'Ruc2hxd'
PASSWORD = 'ruch1232'

led0 = Pin(2, Pin.OUT)
led1 = Pin(4, Pin.OUT)

# ====== Connect WiFi ======
wlan = network.WLAN(network.STA_IF)
wlan.active(True)
if not wlan.isconnected():
    print('Connecting to Wi-Fi...')
    wlan.connect(SSID, PASSWORD)
    while not wlan.isconnected():
        time.sleep(1)
print('Connected! IP:', wlan.ifconfig()[0])

# ====== Socket ======
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
s.bind(('', 80))
s.listen(3)
print('Server ready.')

# ====== Send chunk ======
def send_chunk(conn, data):
    size = 256
    for i in range(0, len(data), size):
        conn.send(data[i:i+size])

# ====== CSS (แยกไว้ลด RAM ======
CSS = (
    '<style>'
    '*{margin:0;padding:0;box-sizing:border-box}'
    'body{background:#000;font-family:"Courier New",monospace;color:#0ff;'
    'min-height:100vh;display:flex;align-items:center;justify-content:center}'
    'body::before{content:"";position:fixed;inset:0;'
    'background:repeating-linear-gradient(0deg,transparent,transparent 2px,rgba(0,255,255,0.02) 2px,rgba(0,255,255,0.02) 4px);'
    'pointer-events:none;z-index:0}'
    '.w{position:relative;z-index:1;width:100%;max-width:400px;padding:1.2rem}'
    'h1{font-size:20px;letter-spacing:5px;text-align:center;margin-bottom:4px;'
    'color:#0ff;text-shadow:0 0 10px #0ff}'
    '.sub{font-size:10px;letter-spacing:4px;color:rgba(0,255,255,0.4);text-align:center;margin-bottom:1.2rem}'
    'hr{border:none;border-top:1px solid rgba(0,255,255,0.15);margin-bottom:1rem}'
    '.card{background:#060606;border:1px solid rgba(0,255,255,0.15);padding:1rem;'
    'margin-bottom:.8rem;position:relative}'
    '.on{border-color:rgba(0,255,65,0.4);box-shadow:0 0 12px rgba(0,255,65,0.15)}'
    '.card::before{content:"";position:absolute;top:0;left:0;width:2px;height:100%;background:#0ff}'
    '.on::before{background:#0f0;box-shadow:0 0 6px #0f0}'
    '.lbl{font-size:9px;letter-spacing:3px;color:rgba(0,255,255,0.5);margin-bottom:.5rem}'
    '.row{display:flex;align-items:center;gap:8px;margin-bottom:.8rem}'
    '.dot{width:9px;height:9px;border-radius:50%;background:#1a1a1a;flex-shrink:0}'
    '.on .dot{background:#0f0;box-shadow:0 0 7px #0f0}'
    '.sv{font-size:16px;color:#333}'
    '.on .sv{color:#0f0;text-shadow:0 0 5px rgba(0,255,65,0.5)}'
    '.btns{display:flex;gap:6px}'
    'a{flex:1;display:block;text-align:center;text-decoration:none;'
    'padding:6px 0;font-family:"Courier New",monospace;font-size:11px;'
    'letter-spacing:2px;border:1px solid}'
    '.bon{border-color:rgba(0,255,65,0.5);color:#0f0}'
    '.boff{border-color:rgba(255,0,60,0.4);color:#f00}'
    '.foot{text-align:center;font-size:9px;color:rgba(0,255,255,0.25);'
    'letter-spacing:3px;margin-top:.8rem}'
    '</style>'
)

# ====== HTML ======
def web_page():
    s0 = 'ON' if led0.value() else 'OFF'
    s1 = 'ON' if led1.value() else 'OFF'
    c0 = ' on' if led0.value() else ''
    c1 = ' on' if led1.value() else ''
    return (
        '<!DOCTYPE html><html><head>'
        '<meta charset="UTF-8">'
        '<meta name="viewport" content="width=device-width,initial-scale=1">'
        '<title>CYBER_CTRL</title>'
        + CSS +
        '</head><body><div class="w">'
        '<h1>// CYBER_CTRL //</h1>'
        '<p class="sub">MicroPython U2 &mdash; LED Interface</p>'
        '<hr>'
        '<div class="card' + c0 + '">'
        '<div class="lbl">[ LED_0 ]</div>'
        '<div class="row"><div class="dot"></div>'
        '<div class="sv">' + s0 + '</div></div>'
        '<div class="btns">'
        '<a class="bon" href="/?led0=on">[ ON ]</a>'
        '<a class="boff" href="/?led0=off">[ OFF ]</a>'
        '</div></div>'
        '<div class="card' + c1 + '">'
        '<div class="lbl">[ LED_1 ]</div>'
        '<div class="row"><div class="dot"></div>'
        '<div class="sv">' + s1 + '</div></div>'
        '<div class="btns">'
        '<a class="bon" href="/?led1=on">[ ON ]</a>'
        '<a class="boff" href="/?led1=off">[ OFF ]</a>'
        '</div></div>'
        '<div class="foot">// NODE: ONLINE //</div>'
        '</div></body></html>'
    )

# ====== Main Loop ======
while True:
    try:
        conn, addr = s.accept()
        request = conn.recv(512).decode('utf-8')

        if   '/?led0=on'  in request: led0.value(1)
        elif '/?led0=off' in request: led0.value(0)
        if   '/?led1=on'  in request: led1.value(1)
        elif '/?led1=off' in request: led1.value(0)

        html = web_page()
        conn.send('HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n')
        send_chunk(conn, html)
        conn.close()
        print('OK -', addr[0], '| L0:', led0.value(), 'L1:', led1.value())

    except Exception as e:
        print('ERR:', e)
        try:
            conn.close()
        except:
            pass