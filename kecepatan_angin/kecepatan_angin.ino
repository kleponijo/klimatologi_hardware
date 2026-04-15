#include <WiFi.h>
#include "config.h"
#include "firebase_helper.h"

// Objek Firebase wajib ada di file utama
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

volatile int pulseCount = 0;
float kecepatan = 0;
unsigned long lastSecond = 0;
unsigned long lastHistory = 0; // Untuk history

void IRAM_ATTR hitungPulsa() {
  pulseCount++;
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_HALL, INPUT_PULLUP);
  pinMode(PIN_BUZZER, OUTPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");

    // Buzzer bunyi 2 kali
    for(int i = 0; i < 2; i++) {
      tone(PIN_BUZZER, 1000);
      delay(300); // bunyi 0.3 detik

      noTone(PIN_BUZZER);
      delay(700); // jeda supaya total 1 detik per beep
    }

    delay(5000);
    
  }

  tone(PIN_BUZZER, 1000);
  delay(500);
  noTone(PIN_BUZZER);
  Serial.println("\nWiFi Connected!");
  Serial.println(WiFi.localIP());

   // Panggil fungsi setup Firebase dari helper
  setupFirebase(fbdo, auth, config);

  attachInterrupt(digitalPinToInterrupt(PIN_HALL), hitungPulsa, FALLING);

  Serial.println("Sistem Anemometer Aktif");
}

void loop() {

  // Logika realtime
  if (millis() - lastSecond >= INTERVAL_REALTIME) {

    int Rdetik = pulseCount;
    pulseCount = 0;

    kecepatan = 6.7824 * Rdetik * K_FAKTOR  ;

    Serial.print("Realtime km/h: ");
    Serial.println(kecepatan);

    // Panggil fungsi sendRealtime dari helper
    sendRealtime(fbdo, kecepatan);

    lastSecond = millis();
  }

   // --- 2. LOGIKA HISTORY ---
  if (millis() - lastHistory >= INTERVAL_HISTORY) {
    // Panggil fungsi yang ada di firebase_helper.h
    sendHistory(fbdo, kecepatan); 
    
    lastHistory = millis(); // Reset waktu history
  }
}