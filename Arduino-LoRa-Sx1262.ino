#include "LoraSx1262.h"

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Booted");

  LoraSx1262 radio = LoraSx1262();
}

void loop() {
  // put your main code here, to run repeatedly:

  Serial.println("Loop");
  delay(1000);
}
