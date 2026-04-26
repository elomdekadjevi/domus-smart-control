#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>

/* ================= WIFI ================= */
const char* ssid = "ESP32-RGB";
const char* password = "12345678";
WebServer server(80);

/* ================= RGB ================= */
#define RED_PIN   25
#define GREEN_PIN 26
#define BLUE_PIN  27

bool rgbLoop = false;
unsigned long previousMillis = 0;
int stepValue = 0;
int phase = 0;

/* ================= RFID ================= */
#define SS_PIN  5
#define RST_PIN 4
MFRC522 rfid(SS_PIN, RST_PIN);

byte carteAutorisee[4] = {0x45, 0xC5, 0x1E, 0x03};

/* ================= SERVO ================= */
#define SERVO_PIN 13
Servo porte;

/* ================= ALARME ================= */
#define PIR_PIN 14
#define BUZZER  12
bool alarmeActive = false;

/* ================= RGB ================= */
void setColor(int r, int g, int b) {
  ledcWrite(RED_PIN, r);
  ledcWrite(GREEN_PIN, g);
  ledcWrite(BLUE_PIN, b);
}

void allOff() {
  setColor(0, 0, 0);
}

/* ================= RFID ================= */
bool isAuthorized(byte *uid) {
  for (byte i = 0; i < 4; i++) {
    if (uid[i] != carteAutorisee[i]) return false;
  }
  return true;
}

/* ================= WEB ================= */
void handleRoot() {
  server.send(200, "text/html", pageHTML());
}

void handleCmd() {
  String cmd = server.arg("cmd");

  if (cmd == "off") {
    rgbLoop = false;
    allOff();
  }
  else if (cmd == "r") {
    rgbLoop = false;
    setColor(255,0,0);
  }
  else if (cmd == "g") {
    rgbLoop = false;
    setColor(0,255,0);
  }
  else if (cmd == "b") {
    rgbLoop = false;
    setColor(0,0,255);
  }
  else if (cmd == "w") {
    rgbLoop = false;
    setColor(255,255,255);
  }
  else if (cmd == "loop") {
    stepValue = 0;
    phase = 0;
    rgbLoop = true;
  }
  else if (cmd == "alarm_on") {
    alarmeActive = true;
  }
  else if (cmd == "alarm_off") {
    alarmeActive = false;
    digitalWrite(BUZZER, LOW);
  }

  server.send(200, "text/plain", "OK");
}

/* ================= SETUP ================= */
void setup() {
  Serial.begin(115200);
  delay(1000);

  /* RGB PWM */
  ledcAttach(RED_PIN,   5000, 8);
  ledcAttach(GREEN_PIN, 5000, 8);
  ledcAttach(BLUE_PIN,  5000, 8);
  allOff();

  /* SERVO */
  porte.setPeriodHertz(50);
  porte.attach(SERVO_PIN, 500, 2400);
  porte.write(0);
  delay(500);
  
  // Test servo au démarrage
  Serial.println("Test servo...");
  porte.write(90);
  delay(1000);
  porte.write(0);
  delay(500);

  /* ALARME */
  pinMode(PIR_PIN, INPUT);
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);

  /* RFID */
  SPI.begin();
  rfid.PCD_Init();
  delay(100);
  Serial.println("RFID initialisé");

  /* WIFI AP */
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  Serial.print("IP ESP32 : ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/cmd", handleCmd);
  server.begin();
  
  Serial.println("=== SYSTEME PRET ===");
}

/* ================= LOOP ================= */
void loop() {
  server.handleClient();

  /* RGB LOOP */
  if (rgbLoop && millis() - previousMillis >= 20) {
    previousMillis = millis();
    switch (phase) {
      case 0: setColor(255-stepValue, stepValue, 0); break;
      case 1: setColor(0, 255-stepValue, stepValue); break;
      case 2: setColor(stepValue, 0, 255-stepValue); break;
    }
    stepValue++;
    if (stepValue > 255) {
      stepValue = 0;
      phase = (phase + 1) % 3;
    }
  }

  /* RFID → SERVO */
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    
    Serial.print("Carte détectée: ");
    for (byte i = 0; i < 4; i++) {
      Serial.print(rfid.uid.uidByte[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
    
    if (isAuthorized(rfid.uid.uidByte)) {
      Serial.println(">>> CARTE AUTORISEE <<<");
      
      // Arrêter RGB pendant l'ouverture
      bool rgbWasActive = rgbLoop;
      rgbLoop = false;
      allOff();
      
      // Ouvrir porte
      porte.write(90);
      Serial.println("Porte ouverte");
      delay(3000);
      
      // Fermer porte
      porte.write(0);
      Serial.println("Porte fermée");
      
      // Restaurer RGB si nécessaire
      if (rgbWasActive) {
        rgbLoop = true;
      }
      
    } else {
      Serial.println(">>> CARTE REFUSEE <<<");
    }
    
    rfid.PICC_HaltA();
    delay(500); // Anti-rebond
  }

  /* ALARME */
  if (alarmeActive) {
    if (digitalRead(PIR_PIN) == HIGH) {
      digitalWrite(BUZZER, HIGH);
      Serial.println("[ALARME] Mouvement détecté!");
    } else {
      digitalWrite(BUZZER, LOW);
    }
  } else {
    digitalWrite(BUZZER, LOW);
  }
}

/* ================= HTML ================= */
String pageHTML() {
  return R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
body{background:#111;color:white;text-align:center;font-family:Arial;padding:20px}
h2{color:#0ff}
button{width:80%;max-width:300px;padding:15px;margin:8px;font-size:18px;border-radius:10px;border:none;cursor:pointer}
button:active{transform:scale(0.95)}
.red{background:red}.green{background:green}
.blue{background:blue}.white{background:#ccc;color:black}
.loop{background:purple}.off{background:#555}
.alarm{background:crimson}
</style>
</head>
<body>

<h2>🏠 DOMUS SMART CONTROL</h2>

<h3>💡 RGB</h3>
<button class="red" onclick="send('r')">Rouge</button>
<button class="green" onclick="send('g')">Vert</button>
<button class="blue" onclick="send('b')">Bleu</button>
<button class="white" onclick="send('w')">Blanc</button>
<button class="loop" onclick="send(
