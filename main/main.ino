#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>

const char* ssid = "klepon";
const char* password = "12345678";

const char* firebaseHost = "https://klimatologiot-default-rtdb.asia-southeast1.firebasedatabase.app/";

const int pinHallEffect = 27;

volatile int pulseCount = 0;

float K = 1.0;
float kecepatan = 0;

// realtime 1 detik
unsigned long lastSecond = 0;
const unsigned long intervalSecond = 1000;

// history 1 jam
unsigned long lastHour = 0;
const unsigned long intervalHour = 3600000;

// akumulasi rata-rata
float totalKecepatan = 0;
int jumlahSample = 0;

void IRAM_ATTR hitungPulsa() {
  pulseCount++;
}

void setup() {
  Serial.begin(115200);
  pinMode(pinHallEffect, INPUT_PULLUP);

  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");
  Serial.println(WiFi.localIP());

  // Sinkron waktu NTP
  configTime(7 * 3600, 0, "pool.ntp.org"); // GMT+7 Indonesia

  attachInterrupt(digitalPinToInterrupt(pinHallEffect), hitungPulsa, FALLING);

  Serial.println("Sistem Anemometer Aktif");
}

void loop() {

  // =========================
  // REALTIME PER DETIK
  // =========================
  if (millis() - lastSecond >= intervalSecond) {

    int Rdetik = pulseCount;
    pulseCount = 0;

    kecepatan = 6.7824 * Rdetik * K;

    totalKecepatan += kecepatan;
    jumlahSample++;

    Serial.print("Realtime km/h: ");
    Serial.println(kecepatan);

    kirimRealtime(kecepatan, Rdetik);

    lastSecond = millis();
  }

  // =========================
  // HISTORY PER JAM
  // =========================
  if (millis() - lastHour >= intervalHour) {

    if (jumlahSample > 0) {

      float rataRata = totalKecepatan / jumlahSample;

      Serial.print("Rata-rata 1 jam: ");
      Serial.println(rataRata);

      kirimHistory(rataRata, jumlahSample);

      totalKecepatan = 0;
      jumlahSample = 0;
    }

    lastHour = millis();
  }
}

// =========================
// FUNCTION REALTIME (PUT)
// =========================
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

// =========================
// FUNCTION HISTORY (POST)
// =========================
void kirimHistory(float rataRata, int totalSample) {

  if (WiFi.status() == WL_CONNECTED) {

    HTTPClient http;
    String url = String(firebaseHost) + "/anemometer/history_per_jam.json";

    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    time_t now = time(nullptr);

    String jsonData = "{";
    jsonData += "\"rata_rata\":" + String(rataRata, 2) + ",";
    jsonData += "\"total_sample\":" + String(totalSample) + ",";
    jsonData += "\"timestamp\":" + String(now);
    jsonData += "}";

    int response = http.POST(jsonData);

    Serial.print("History HTTP: ");
    Serial.println(response);

    http.end();
  }
}