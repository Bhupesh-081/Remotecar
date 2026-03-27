#include <WiFi.h>
#include <WebServer.h>

// Access point credentials.
const char *kSsid = "PatrolCarLab";
const char *kPassword = "Patrol1234";

// L298N dual motor driver pins.
const int kEnA = 25;
const int kIn1 = 26;
const int kIn2 = 27;
const int kEnB = 14;
const int kIn3 = 12;
const int kIn4 = 13;

// Sensor and utility pins.
const int kTrigPin = 5;
const int kEchoPin = 18;
const int kBatteryAdcPin = 34;
const int kHeadlightPin = 2;

const int kPwmChannelA = 0;
const int kPwmChannelB = 1;
const int kPwmFrequency = 20000;
const int kPwmResolutionBits = 8;

enum MotionState {
  STOPPED,
  FORWARD,
  BACKWARD,
  LEFT,
  RIGHT
};

WebServer server(80);

MotionState motion = STOPPED;
int speedValue = 180;
bool patrolEnabled = false;
bool headlightsEnabled = false;

unsigned long lastPatrolToggleMs = 0;
bool patrolTurnPhase = false;

bool avoidanceActive = false;
unsigned long avoidanceUntilMs = 0;
int avoidanceStage = 0;

const char kDashboardHtml[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>ESP32 Patrol Console</title>
  <style>
    :root {
      --bg: #101820;
      --surface: #1f2a36;
      --surface-soft: #2b3b4b;
      --accent: #ff6b35;
      --accent-soft: #ff9f7f;
      --ok: #31d0aa;
      --text: #f5f7fa;
      --muted: #b8c4cf;
    }
    * { box-sizing: border-box; }
    body {
      margin: 0;
      font-family: "Trebuchet MS", "Segoe UI", sans-serif;
      color: var(--text);
      background:
        radial-gradient(circle at 20% 10%, #314257 0%, transparent 40%),
        radial-gradient(circle at 80% 90%, #28374a 0%, transparent 45%),
        var(--bg);
      min-height: 100vh;
      display: grid;
      place-items: center;
      padding: 20px;
    }
    .panel {
      width: min(900px, 100%);
      background: linear-gradient(180deg, rgba(255,255,255,0.06), rgba(255,255,255,0.02));
      border: 1px solid rgba(255,255,255,0.12);
      border-radius: 20px;
      padding: 24px;
      backdrop-filter: blur(6px);
      box-shadow: 0 18px 32px rgba(0,0,0,0.35);
    }
    h1 {
      margin: 0 0 8px 0;
      letter-spacing: 0.08em;
      text-transform: uppercase;
      font-size: 1.5rem;
    }
    .sub {
      color: var(--muted);
      margin-bottom: 20px;
      font-size: 0.95rem;
    }
    .grid {
      display: grid;
      gap: 16px;
      grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
    }
    .card {
      background: var(--surface);
      border: 1px solid rgba(255,255,255,0.08);
      border-radius: 14px;
      padding: 14px;
    }
    .label {
      font-size: 0.75rem;
      color: var(--muted);
      text-transform: uppercase;
      letter-spacing: 0.08em;
    }
    .value {
      margin-top: 6px;
      font-weight: 700;
      font-size: 1.35rem;
    }
    .controls {
      margin-top: 20px;
      display: grid;
      gap: 12px;
    }
    .drive-pad {
      display: grid;
      grid-template-columns: repeat(3, minmax(70px, 1fr));
      gap: 8px;
      max-width: 360px;
      margin: 0 auto;
    }
    button {
      border: none;
      border-radius: 10px;
      padding: 12px 10px;
      color: var(--text);
      background: var(--surface-soft);
      font-weight: 700;
      cursor: pointer;
      transition: transform 120ms ease, background 120ms ease;
    }
    button:hover { transform: translateY(-2px); }
    .action { background: var(--accent); color: #161616; }
    .ok { background: var(--ok); color: #0b1e18; }
    .slider-wrap {
      display: grid;
      gap: 6px;
      max-width: 360px;
      margin: 0 auto;
    }
    input[type="range"] { width: 100%; }
    @media (max-width: 520px) {
      .panel { padding: 16px; }
      .value { font-size: 1.15rem; }
    }
  </style>
</head>
<body>
  <section class="panel">
    <h1>Patrol Car Command Center</h1>
    <div class="sub">Final-year style dashboard on ESP32 soft AP</div>

    <div class="grid">
      <article class="card"><div class="label">Motion</div><div class="value" id="motion">-</div></article>
      <article class="card"><div class="label">Speed</div><div class="value" id="speed">-</div></article>
      <article class="card"><div class="label">Distance (cm)</div><div class="value" id="distance">-</div></article>
      <article class="card"><div class="label">Battery</div><div class="value" id="battery">-</div></article>
      <article class="card"><div class="label">RSSI</div><div class="value" id="rssi">-</div></article>
      <article class="card"><div class="label">Uptime</div><div class="value" id="uptime">-</div></article>
    </div>

    <div class="controls">
      <div class="drive-pad">
        <div></div>
        <button class="action" onclick="drive('forward')">Forward</button>
        <div></div>
        <button class="action" onclick="drive('left')">Left</button>
        <button onclick="drive('stop')">Stop</button>
        <button class="action" onclick="drive('right')">Right</button>
        <div></div>
        <button class="action" onclick="drive('backward')">Backward</button>
        <div></div>
      </div>

      <div class="slider-wrap">
        <label for="spd">Speed</label>
        <input id="spd" type="range" min="80" max="255" value="180" oninput="setSpeed(this.value)">
      </div>

      <div class="drive-pad" style="max-width:560px;grid-template-columns:repeat(2,minmax(120px,1fr));">
        <button class="ok" onclick="setPatrol(true)">Enable Patrol</button>
        <button onclick="setPatrol(false)">Disable Patrol</button>
        <button class="ok" onclick="setHeadlight(true)">Headlights On</button>
        <button onclick="setHeadlight(false)">Headlights Off</button>
      </div>
    </div>
  </section>

  <script>
    async function drive(dir) {
      await fetch('/api/drive?dir=' + dir);
      await refresh();
    }

    async function setSpeed(value) {
      await fetch('/api/speed?value=' + value);
      document.getElementById('speed').textContent = value;
    }

    async function setPatrol(enable) {
      await fetch('/api/patrol?enable=' + (enable ? '1' : '0'));
      await refresh();
    }

    async function setHeadlight(enable) {
      await fetch('/api/headlight?enable=' + (enable ? '1' : '0'));
      await refresh();
    }

    async function refresh() {
      const response = await fetch('/api/status');
      const status = await response.json();
      document.getElementById('motion').textContent = status.motion + (status.patrol ? ' | Patrol' : '');
      document.getElementById('speed').textContent = status.speed;
      document.getElementById('distance').textContent = status.distance_cm;
      document.getElementById('battery').textContent = status.battery_pct + '%';
      document.getElementById('rssi').textContent = status.rssi_dbm + ' dBm';
      document.getElementById('uptime').textContent = status.uptime_s + ' s';
      document.getElementById('spd').value = status.speed;
    }

    setInterval(refresh, 700);
    refresh();
  </script>
</body>
</html>
)rawliteral";

String motionToString(MotionState value) {
  switch (value) {
    case FORWARD: return "forward";
    case BACKWARD: return "backward";
    case LEFT: return "left";
    case RIGHT: return "right";
    case STOPPED:
    default: return "stopped";
  }
}

float readDistanceCm() {
  digitalWrite(kTrigPin, LOW);
  delayMicroseconds(3);
  digitalWrite(kTrigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(kTrigPin, LOW);

  long pulse = pulseIn(kEchoPin, HIGH, 30000);
  if (pulse <= 0) {
    return -1.0f;
  }
  return pulse * 0.0343f * 0.5f;
}

int readBatteryPercent() {
  int raw = analogRead(kBatteryAdcPin);
  float voltage = (raw / 4095.0f) * 3.3f * 2.0f;
  float percent = ((voltage - 6.4f) / (8.4f - 6.4f)) * 100.0f;
  if (percent < 0.0f) percent = 0.0f;
  if (percent > 100.0f) percent = 100.0f;
  return static_cast<int>(percent + 0.5f);
}

void applyMotor(int leftMotor, int rightMotor) {
  auto driveOne = [](int speedRaw, int inA, int inB, int channel) {
    bool forward = speedRaw >= 0;
    int pwm = abs(speedRaw);
    if (pwm > 255) pwm = 255;
    digitalWrite(inA, forward ? HIGH : LOW);
    digitalWrite(inB, forward ? LOW : HIGH);
    ledcWrite(channel, pwm);
  };

  driveOne(leftMotor, kIn1, kIn2, kPwmChannelA);
  driveOne(rightMotor, kIn3, kIn4, kPwmChannelB);
}

void setMotion(MotionState next) {
  motion = next;
  switch (next) {
    case FORWARD:
      applyMotor(speedValue, speedValue);
      break;
    case BACKWARD:
      applyMotor(-speedValue, -speedValue);
      break;
    case LEFT:
      applyMotor(-speedValue, speedValue);
      break;
    case RIGHT:
      applyMotor(speedValue, -speedValue);
      break;
    case STOPPED:
    default:
      applyMotor(0, 0);
      break;
  }
}

void handleRoot() {
  server.send(200, "text/html", kDashboardHtml);
}

void handleDrive() {
  String dir = server.arg("dir");
  patrolEnabled = false;
  avoidanceActive = false;

  if (dir == "forward") {
    setMotion(FORWARD);
  } else if (dir == "backward") {
    setMotion(BACKWARD);
  } else if (dir == "left") {
    setMotion(LEFT);
  } else if (dir == "right") {
    setMotion(RIGHT);
  } else {
    setMotion(STOPPED);
  }

  server.send(200, "application/json", "{\"ok\":true}");
}

void handleSpeed() {
  int requested = server.arg("value").toInt();
  if (requested < 80) requested = 80;
  if (requested > 255) requested = 255;
  speedValue = requested;
  setMotion(motion);
  server.send(200, "application/json", "{\"ok\":true}");
}

void handlePatrol() {
  patrolEnabled = server.arg("enable") == "1";
  lastPatrolToggleMs = millis();
  patrolTurnPhase = false;
  avoidanceActive = false;
  setMotion(patrolEnabled ? FORWARD : STOPPED);
  server.send(200, "application/json", "{\"ok\":true}");
}

void handleHeadlight() {
  headlightsEnabled = server.arg("enable") == "1";
  digitalWrite(kHeadlightPin, headlightsEnabled ? HIGH : LOW);
  server.send(200, "application/json", "{\"ok\":true}");
}

void handleStatus() {
  float distance = readDistanceCm();
  int batteryPct = readBatteryPercent();
  long rssi = WiFi.RSSI();
  unsigned long uptime = millis() / 1000;

  String payload = "{";
  payload += "\"motion\":\"" + motionToString(motion) + "\",";
  payload += "\"speed\":" + String(speedValue) + ",";
  payload += "\"distance_cm\":" + String(distance < 0 ? 0 : distance, 1) + ",";
  payload += "\"battery_pct\":" + String(batteryPct) + ",";
  payload += "\"rssi_dbm\":" + String(rssi) + ",";
  payload += "\"patrol\":" + String(patrolEnabled ? "true" : "false") + ",";
  payload += "\"headlights\":" + String(headlightsEnabled ? "true" : "false") + ",";
  payload += "\"uptime_s\":" + String(uptime);
  payload += "}";

  server.send(200, "application/json", payload);
}

void handleNotFound() {
  server.send(404, "application/json", "{\"ok\":false,\"message\":\"route not found\"}");
}

void runPatrolController() {
  if (!patrolEnabled) {
    return;
  }

  float distance = readDistanceCm();
  unsigned long now = millis();

  if (!avoidanceActive && distance > 0 && distance < 18.0f) {
    avoidanceActive = true;
    avoidanceStage = 0;
    avoidanceUntilMs = now + 350;
    setMotion(BACKWARD);
    return;
  }

  if (avoidanceActive) {
    if (now < avoidanceUntilMs) {
      return;
    }
    if (avoidanceStage == 0) {
      avoidanceStage = 1;
      avoidanceUntilMs = now + 450;
      setMotion(RIGHT);
      return;
    }

    avoidanceActive = false;
    setMotion(FORWARD);
    lastPatrolToggleMs = now;
    patrolTurnPhase = false;
    return;
  }

  if (now - lastPatrolToggleMs < 2200) {
    return;
  }

  lastPatrolToggleMs = now;
  patrolTurnPhase = !patrolTurnPhase;
  setMotion(patrolTurnPhase ? LEFT : FORWARD);
}

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(kIn1, OUTPUT);
  pinMode(kIn2, OUTPUT);
  pinMode(kIn3, OUTPUT);
  pinMode(kIn4, OUTPUT);
  pinMode(kTrigPin, OUTPUT);
  pinMode(kEchoPin, INPUT);
  pinMode(kHeadlightPin, OUTPUT);
  digitalWrite(kHeadlightPin, LOW);

  ledcSetup(kPwmChannelA, kPwmFrequency, kPwmResolutionBits);
  ledcSetup(kPwmChannelB, kPwmFrequency, kPwmResolutionBits);
  ledcAttachPin(kEnA, kPwmChannelA);
  ledcAttachPin(kEnB, kPwmChannelB);

  analogReadResolution(12);
  setMotion(STOPPED);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(kSsid, kPassword);
  delay(200);

  Serial.println();
  Serial.println("Patrol Car AP started");
  Serial.print("SSID: ");
  Serial.println(kSsid);
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/api/drive", HTTP_GET, handleDrive);
  server.on("/api/speed", HTTP_GET, handleSpeed);
  server.on("/api/patrol", HTTP_GET, handlePatrol);
  server.on("/api/headlight", HTTP_GET, handleHeadlight);
  server.on("/api/status", HTTP_GET, handleStatus);
  server.onNotFound(handleNotFound);
  server.begin();
}

void loop() {
  server.handleClient();
  runPatrolController();
}
