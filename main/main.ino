int ledPin;
String text = "Hello World!";

void setup() {
  serial.printl(text);
  serial.begin(9600);
}

void loop() {

}