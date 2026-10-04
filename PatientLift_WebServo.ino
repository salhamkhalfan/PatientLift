#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

// ---------- Settings ----------
const char* AP_SSID = "PatientLift";
const char* AP_PASS = "change-me-1234";   // min 8 chars. CHANGE THIS.

const int   SERVO_PIN     = 18;
const int   MIN_ANGLE     = 0;
const int   MAX_ANGLE     = 110;
const int   START_ANGLE   = 90;
const float MAX_SPEED_DPS = 30.0;   // max degrees per second (smooth, gentle motion)
const unsigned long STEP_MS = 20;   // motion update interval

// ---------- State ----------
Servo myServo;
WebServer server(80);
float currentAngle = START_ANGLE;
int   targetAngle  = START_ANGLE;
unsigned long lastStep = 0;

// ---------- Web page ----------
const char PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en"><head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Patient Lift Control</title>
<style>
:root{--bg:#f2f4f3;--ink:#1b2a2f;--mute:#5d6b70;--line:#c9d2d4;--acc:#0b6e6e;--stop:#b3261e;--card:#fff}
@media (prefers-color-scheme:dark){:root{--bg:#121a1c;--ink:#e8eeef;--mute:#9aa9ad;--line:#2c3b3f;--acc:#4fc3c3;--card:#1a2427}}
*{box-sizing:border-box}
body{margin:0;background:var(--bg);color:var(--ink);font-family:system-ui,-apple-system,"Segoe UI",Roboto,sans-serif;display:flex;justify-content:center}
main{width:100%;max-width:480px;padding:16px 16px 32px}
h1{font-size:1.1rem;margin:4px 0 12px;font-weight:600}
.conn{font-size:.85rem;color:var(--mute);margin-bottom:12px}
.conn.bad{color:var(--stop);font-weight:600}
.read{background:var(--card);border:1px solid var(--line);border-radius:12px;padding:16px;text-align:center}
.num{font-size:4.5rem;font-weight:700;line-height:1;font-variant-numeric:tabular-nums}
.state{color:var(--mute);margin-top:6px;min-height:1.4em}
.bar{height:10px;background:var(--line);border-radius:5px;margin-top:12px;overflow:hidden}
.bar i{display:block;height:100%;width:0;background:var(--acc);transition:width .2s}
.row{display:flex;gap:10px;margin-top:14px}
.row>*{flex:1}
button{font:inherit;font-size:1.1rem;min-height:56px;border-radius:10px;border:1px solid var(--line);background:var(--card);color:var(--ink)}
button{cursor:pointer;font-weight:600}
button:active{transform:scale(.98)}
button:focus-visible{outline:3px solid var(--acc);outline-offset:2px}
.steps button{min-height:96px;font-size:2rem}
#stop{width:100%;margin-top:18px;min-height:76px;font-size:1.5rem;background:var(--stop);border-color:var(--stop);color:#fff}
.msg{min-height:1.4em;margin-top:10px;color:var(--stop);font-size:.95rem}
</style></head>
<body><main>
<h1>Patient Lift Control</h1>
<div id="conn" class="conn">Connecting…</div>

<section class="read">
  <div class="num"><span id="cur">--</span>&deg;</div>
  <div class="state" id="state">&nbsp;</div>
  <div class="bar"><i id="fill"></i></div>
</section>

<div class="row steps">
  <button id="dec">&minus; 5&deg;</button>
  <button id="inc">+ 5&deg;</button>
</div>

<button id="stop">STOP</button>
<div class="msg" id="msg" role="alert"></div>
</main>
<script>
const $=id=>document.getElementById(id);
const MIN=0, MAX=110;

function msg(t){$('msg').textContent=t||'';}
function conn(ok){const c=$('conn');c.className='conn'+(ok?'':' bad');c.textContent=ok?'Connected':'Connection lost – controller not responding';}

function render(s){
  if(s.current===undefined)return;
  $('cur').textContent=s.current;
  $('fill').style.width=(100*(s.current-MIN)/(MAX-MIN))+'%';
  $('state').textContent=(s.current===s.target)?'Holding position':'Moving to '+s.target+'°';
}

async function call(url){
  try{
    const r=await fetch(url,{cache:'no-store'});
    const j=await r.json();
    conn(true);
    if(!r.ok){msg(j.error||'Command rejected');}else{msg('');}
    render(j);
  }catch(e){conn(false);}
}

$('inc').onclick=()=>call('/step?d=5');
$('dec').onclick=()=>call('/step?d=-5');
$('stop').onclick=()=>call('/stop');

setInterval(()=>call('/status'),500);
call('/status');
</script></body></html>
)rawliteral";

// ---------- Motion ----------
void setTarget(int a) {
  targetAngle = constrain(a, MIN_ANGLE, MAX_ANGLE);
}

void updateMotion() {
  unsigned long now = millis();
  if (now - lastStep < STEP_MS) return;
  float dt = (now - lastStep) / 1000.0;
  lastStep = now;

  float maxStep = MAX_SPEED_DPS * dt;
  float diff = targetAngle - currentAngle;

  if (fabs(diff) <= maxStep) currentAngle = targetAngle;
  else currentAngle += (diff > 0 ? maxStep : -maxStep);

  myServo.write((int)round(currentAngle));
}

// ---------- HTTP handlers ----------
bool isWholeNumber(const String& s) {
  if (s.length() == 0) return false;
  for (unsigned int i = 0; i < s.length(); i++) {
    if (i == 0 && s[i] == '-' && s.length() > 1) continue;
    if (!isDigit(s[i])) return false;
  }
  return true;
}

void sendStatus(int code = 200, const char* error = nullptr) {
  String j = "{\"current\":" + String((int)round(currentAngle)) +
             ",\"target\":" + String(targetAngle) +
             ",\"min\":" + String(MIN_ANGLE) +
             ",\"max\":" + String(MAX_ANGLE);
  if (error) j += ",\"error\":\"" + String(error) + "\"";
  j += "}";
  server.sendHeader("Cache-Control", "no-store");
  server.send(code, "application/json", j);
}

void handleRoot()   { server.send_P(200, "text/html", PAGE); }
void handleStatus() { sendStatus(); }

void handleStep() {
  String d = server.arg("d");
  if (!isWholeNumber(d)) { sendStatus(400, "Invalid step"); return; }
  int step = constrain(d.toInt(), -10, 10);   // limit step size
  setTarget(targetAngle + step);
  Serial.printf("Web: step %d -> target %d\n", step, targetAngle);
  sendStatus();
}

void handleStop() {
  targetAngle = (int)round(currentAngle);   // hold where we are
  currentAngle = targetAngle;
  Serial.printf("Web: STOP at %d\n", targetAngle);
  sendStatus();
}

// ---------- Serial (kept from original) ----------
void handleSerial() {
  if (Serial.available() <= 0) return;
  char c = Serial.peek();

  if (c == '+')      { Serial.read(); setTarget(targetAngle + 5); }
  else if (c == '-') { Serial.read(); setTarget(targetAngle - 5); }
  else if (isDigit(c)) {
    int t = Serial.parseInt();
    if (t >= MIN_ANGLE && t <= MAX_ANGLE) setTarget(t);
    else Serial.printf("Error: %d is out of range! Enter %d to %d.\n", t, MIN_ANGLE, MAX_ANGLE);
  }
  else Serial.read();

  while (Serial.available() > 0 &&
         (Serial.peek() == '\r' || Serial.peek() == '\n' || Serial.peek() == ' ')) Serial.read();
}

// ---------- Setup / loop ----------
void setup() {
  Serial.begin(115200);

  ESP32PWM::allocateTimer(0);
  myServo.setPeriodHertz(50);
  myServo.attach(SERVO_PIN, 500, 2400);
  myServo.write(START_ANGLE);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);

  server.on("/", handleRoot);
  server.on("/status", handleStatus);
  server.on("/step", handleStep);
  server.on("/stop", handleStop);
  server.begin();

  Serial.println("\n=== Patient Lift Web Controller ===");
  Serial.printf("Wi-Fi: %s\n", AP_SSID);
  Serial.printf("Open:  http://%s\n", WiFi.softAPIP().toString().c_str());
  Serial.printf("Range: %d to %d deg, max speed %.0f deg/s\n", MIN_ANGLE, MAX_ANGLE, MAX_SPEED_DPS);
}

void loop() {
  server.handleClient();
  handleSerial();
  updateMotion();
}
