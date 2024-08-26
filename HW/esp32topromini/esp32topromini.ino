#define RXD2 16
#define TXD2 17

void setup() {
  // put your setup code here, to run once:
  Serial2.begin(1200, SERIAL_8N1, RXD2, TXD2);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial2.println("Detak 75BPM:NORMAL");
  delay(2000);
  Serial2.println("SPO2 99%:NORMAL");
  delay(2000);
  Serial2.println("SD 130/105:NORMAL");
  delay(2000);
}
