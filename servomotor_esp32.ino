// ESP32 Captive Portal + Servo control (joystick on web page)
// Requirements: Install "ESP32Servo" library in Arduino IDE
// Connections: servo signal -> GPIO 18 (change SERVO_PIN if needed)
// Servo power: use external 5V supply and common ground.

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <ESP32Servo.h>

const char* apSSID = "ESP_SERVO_PORTAL";
const char* apPassword = ""; // open AP. For password, put e.g. "mypassword" and change WiFi.softAP call.
IPAddress apIP(192,168,4,1);
DNSServer dnsServer;
WebServer server(80);

const byte DNS_PORT = 53;

const int SERVO_PIN = 18;
Servo myServo;

int currentAngle = 90; // start at middle

// HTML page served to clients (embedded)
const char index_html[] PROGMEM = R"rawliteral(
<!doctype html>
<html>
<head>
  <meta name="viewport" content="width=device-width,initial-scale=1,user-scalable=no"/>
  <title>ESP32 Servo Joystick</title>
  <style>
    body{font-family:Arial,Helvetica,sans-serif; text-align:center; margin:0; padding:12px; background:#f4f4f8;}
    h1{font-size:20px; margin:8px;}
    #joystick { width:260px; height:260px; margin:12px auto; background:#fff; border-radius:12px; box-shadow: 0 2px 6px rgba(0,0,0,0.15); position:relative; touch-action:none; }
    #knob { width:80px; height:80px; border-radius:50%; position:absolute; left:calc(50% - 40px); top:calc(50% - 40px); background:#2b7cff; display:flex; align-items:center; justify-content:center; color:white; font-weight:bold; user-select:none; }
    #angleDisplay{font-size:18px; margin-top:6px;}
    #slider{ width:90%; margin:14px auto; display:block; }
    small{color:#666;}
    footer{font-size:12px; color:#666; margin-top:8px;}
  </style>
</head>
<body>
  <h1>ESP32 Servo Controller</h1>
  <div id="joystick" aria-label="joystick area">
    <div id="knob">90</div>
  </div>
  <div id="angleDisplay">Angle: <span id="ang">90</span>°</div>
  <input id="slider" type="range" min="0" max="180" value="90">
  <p><small>Drag knob horizontally or use slider. This device works offline as a captive portal.</small></p>
  <footer>ESP32 &mdash; Servo 0&deg; to 180&deg;</footer>

<script>
const knob = document.getElementById('knob');
const area = document.getElementById('joystick');
const angSpan = document.getElementById('ang');
const slider = document.getElementById('slider');

let dragging = false;
let areaRect = area.getBoundingClientRect();

function sendAngle(a){
  angSpan.textContent = a;
  knob.textContent = a;
  // send to ESP
  fetch('/set?angle='+a).catch(e=>{ /* ignore */ });
}

// map x position within area to angle 0-180
function posToAngle(clientX){
  const rect = area.getBoundingClientRect();
  const x = Math.max(0, Math.min(rect.width, clientX - rect.left));
  const angle = Math.round((x / rect.width) * 180);
  return angle;
}

// Pointer events for touch + mouse
area.addEventListener('pointerdown', (ev)=>{
  dragging = true;
  area.setPointerCapture(ev.pointerId);
  const a = posToAngle(ev.clientX);
  // move knob
  moveKnobToAngle(a);
  sendAngle(a);
});

area.addEventListener('pointermove', (ev)=>{
  if(!dragging) return;
  const a = posToAngle(ev.clientX);
  moveKnobToAngle(a);
  sendAngle(a);
});

area.addEventListener('pointerup', (ev)=>{
  dragging = false;
  try { area.releasePointerCapture(ev.pointerId); } catch(e){}
});

// Slider fallback
slider.addEventListener('input', (ev)=>{
  const a = parseInt(ev.target.value);
  moveKnobToAngle(a);
  sendAngle(a);
});

function moveKnobToAngle(angle){
  const rect = area.getBoundingClientRect();
  const knobEl = knob;
  const knobW = knobEl.offsetWidth;
  // compute x position
  const x = (angle / 180) * rect.width;
  const left = Math.max(0, Math.min(rect.width - knobW, x - knobW/2));
  knobEl.style.left = (left) + 'px';
  angSpan.textContent = angle;
  knobEl.textContent = angle;
  slider.value = angle;
}

// On load, request current angle from server
fetch('/current').then(r=>r.text()).then(t=>{
  let a = parseInt(t);
  if(isNaN(a)) a = 90;
  moveKnobToAngle(a);
}).catch(()=>{/*ignore*/});
</script>
</body>
</html>
)rawliteral";

void handleRoot(){
  server.send_P(200, "text/html", index_html);
}

void handleSetAngle(){
  if (server.hasArg("angle")) {
    int a = server.arg("angle").toInt();
    a = constrain(a, 0, 180);
    currentAngle = a;
    myServo.write(currentAngle);
    server.send(200, "text/plain", String(currentAngle));
  } else {
    server.send(400, "text/plain", "missing angle");
  }
}

void handleCurrent(){
  server.send(200, "text/plain", String(currentAngle));
}

void setup() {
  Serial.begin(115200);
  delay(100);

  // Attach servo
  myServo.setPeriodHertz(50); // standard servo
  myServo.attach(SERVO_PIN);
  myServo.write(currentAngle);

  // Start SoftAP
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_AP);
  bool apok = WiFi.softAP(apSSID); // open AP; for password use WiFi.softAP(apSSID, apPassword)
  if(!apok) {
    Serial.println("AP start failed!");
  } else {
    Serial.print("AP started. IP: ");
    Serial.println(WiFi.softAPIP());
  }

  // Set AP IP (optional, ensure AP starts before config)
  WiFi.softAPConfig(apIP, apIP, IPAddress(255,255,255,0));

  // Start DNS server to redirect everything to the ESP
  dnsServer.start(DNS_PORT, "*", apIP);

  // Webserver endpoints
  server.on("/", handleRoot);
  server.on("/set", HTTP_GET, handleSetAngle);
  server.on("/current", HTTP_GET, handleCurrent);
  server.onNotFound([](){
    // redirect to root for any unknown path (makes it captive)
    server.sendHeader("Location", String("http://") + apIP.toString() + "/", true);
    server.send(302, "text/plain", "");
  });
  server.begin();

  Serial.println("HTTP server started");
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
  // nothing else needed
}
