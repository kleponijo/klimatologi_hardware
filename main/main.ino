#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>

const char* ssid = "klepon";
const char* password = "12345678";

const char* firebaseHost = "https://klimatologiot-default-rtdb.asia-southeast1.firebasedatabase.app/";

const int pinHallEffect = 32;
const int pinBuzzer = 18;

volatile int pulseCount = 0;

float K = 1.0;
float kecepatan = 0;

unsigned long lastSecond = 0;
const unsigned long intervalSecond = 1000;

void IRAM_ATTR hitungPulsa() {
  pulseCount++;
}

void setup() {
  Serial.begin(115200);
  pinMode(pinHallEffect, INPUT_PULLUP);
  pinMode(pinBuzzer, OUTPUT);

  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");

    // Buzzer bunyi 2 kali
    for(int i = 0; i < 2; i++) {
      tone(pinBuzzer, 1000);
      delay(300); // bunyi 0.3 detik

      noTone(pinBuzzer);
      delay(700); // jeda supaya total 1 detik per beep
    }

    delay(5000);
    
  }

  tone(pinBuzzer, 1000);
  delay(500);
  noTone(pinBuzzer);
  Serial.println("\nWiFi Connected!");
  Serial.println(WiFi.localIP());

  configTime(7 * 3600, 0, "pool.ntp.org");

  attachInterrupt(digitalPinToInterrupt(pinHallEffect), hitungPulsa, FALLING);

  Serial.println("Sistem Anemometer Aktif");
}

void loop() {

  if (millis() - lastSecond >= intervalSecond) {

    int Rdetik = pulseCount;
    pulseCount = 0;

    kecepatan = 6.7824 * Rdetik * K;

    Serial.print("Realtime km/h: ");
    Serial.println(kecepatan);

    kirimRealtime(kecepatan, Rdetik);

    lastSecond = millis();
  }
}

void kirimRealtime(float kecepatan, int pulsa) {

  if (WiFi.status() == WL_CONNECTED) {

    HTTPClient http;
    String url = String(firebaseHost) + "/anemometer/realtime.json";

    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    time_t now = time(nullptr);

    String jsonData = "{";
    jsonData += "\"kecepatan\":" + String(kecepatan, 2) + ",";
    jsonData += "\"pulsa\":" + String(pulsa) + ",";
    jsonData += "\"timestamp\":" + String(now);
    jsonData += "}";

    int response = http.PUT(jsonData);

    Serial.print("Realtime HTTP: ");
    Serial.println(response);

    http.end();
  }
}