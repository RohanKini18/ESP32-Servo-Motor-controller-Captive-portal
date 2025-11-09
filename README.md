# 🕹️ ESP32 Servo Joystick Captive Portal

This project turns your **ESP32** into a **Wi-Fi Access Point (no internet)** that hosts a simple **web joystick** to control a **servo motor (0°–180°)** in real time.  
No external Wi-Fi network or mobile app is required — connect directly to the ESP32 and move the servo through your browser.

---

## ⚙️ Features
- 🧭 Captive Wi-Fi Portal (auto-opens when you connect)
- 🕹️ On-screen **Joystick & Slider** for servo control
- 🌐 Works completely **offline** — no router needed
- 🔄 Real-time angle updates (0° to 180°)
- ⚡ Simple HTML + JavaScript interface hosted from ESP32
- 🔌 Uses standard **ESP32Servo** library for PWM control

---

## 🧩 Hardware Connections

| Component | ESP32 Pin | Notes |
|------------|-----------|-------|
| **Servo Signal (Orange / White)** | **GPIO 18** | PWM output pin (you can change in code) |
| **Servo VCC (Red)** | **5V (VIN)** | Power supply for servo — use external 5V if possible |
| **Servo GND (Brown / Black)** | **GND** | Common ground with ESP32 |

> ⚠️ Always connect **servo ground** to **ESP32 ground**.  
> For stable operation, add a **470–1000 µF capacitor** between 5V and GND near the servo.

---

### 🧠 Optional External Power Setup
If your servo draws a lot of current (e.g., MG995, MG996R):

```
(External 5V) ───────→ Servo VCC (Red)
(External GND) ──────┬→ Servo GND (Brown)
                     └→ ESP32 GND
ESP32 GPIO 18 ───────→ Servo Signal (Orange)
```

---

## 🧰 Required Libraries
Install these in **Arduino IDE → Sketch → Include Library → Manage Libraries…**

- **ESP32Servo** by Kevin Harrington  
- **WebServer** *(included with ESP32 core)*  
- **DNSServer** *(included with ESP32 core)*  

---

## 💻 How It Works

1. ESP32 starts a Wi-Fi access point named:  
   ```
   ESP_SERVO_PORTAL
   ```
   (Open — no password)
2. It runs a **DNS server** that redirects all sites to the ESP32’s own page (`192.168.4.1`)
3. The browser shows a **joystick webpage** served from the ESP32
4. When you drag the joystick (or use the slider), JavaScript sends requests like:
   ```
   /set?angle=NN
   ```
   where `NN` is the servo angle (0–180)
5. The ESP32 receives the value and moves the servo via PWM.

---

## 🧾 Steps to Run

1. **Connect your ESP32** via USB to your computer.  
2. Open `ESP32_Servo_Portal.ino` in Arduino IDE.  
3. Select correct board:  
   ```
   Tools → Board → ESP32 Dev Module
   ```
4. Upload the sketch.  
5. After upload, open **Serial Monitor** at `115200 baud`.  
   You should see:  
   ```
   AP started. IP: 192.168.4.1
   ```
6. On your phone/laptop, connect to Wi-Fi:  
   ```
   ESP_SERVO_PORTAL
   ```
7. The portal page will automatically open (if not, go to `http://192.168.4.1`)
8. Move the joystick or slider → servo rotates!

---

## 🧩 Troubleshooting

| Issue | Possible Cause | Fix |
|-------|----------------|-----|
| Servo jittering | Weak USB power | Use external 5V adapter |
| Portal not opening automatically | Captive check blocked | Open browser manually → `192.168.4.1` |
| Servo not moving | Wrong GPIO or no common GND | Check wiring |
| Compilation error about `setperiodHertz` | Typo | Use `setPeriodHertz(50)` with capital P |

---

## 📜 License
MIT License — free to use, modify, and share.

---

## 👨‍💻 Author
**T Rohan Kini**  
NMAM Institute of Technology (NMAMIT)  
B.Tech Computer Science & Engineering  
📅 November 2025  

---

[![BuyMeACoffee](https://img.shields.io/badge/Buy%20Me%20a%20Coffee-ffdd00?style=for-the-badge&logo=buy-me-a-coffee&logoColor=black)](https://buymeacoffee.com/rohankini) [![PayPal](https://img.shields.io/badge/PayPal-00457C?style=for-the-badge&logo=paypal&logoColor=white)](https://paypal.me/RohanKinirk) 

