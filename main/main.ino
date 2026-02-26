#include <WiFi.h>

const char* ssid = "DIDIET CH";
const char* password = "didietch";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  Serial.println("connecting");

  int timeout = 20;

  while (WiFi.status() != WL_CONNECTED && timeout > 0) {
    delay(500);
    Serial.print(".");
    timeout--;
  }

  if (WiFi.status() == WL_CONNECTED) {
  Serial.println("Connected!");
  Serial.println(WiFi.localIP());
} else {
  Serial.println("Failed to connect!");
}
  
}

void loop() {

}