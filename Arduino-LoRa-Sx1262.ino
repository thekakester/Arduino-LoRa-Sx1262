/*License: CC 4.0 - Attribution, NonCommercial (by Mitch Davis, github.com/thekakester)
* https://creativecommons.org/licenses/by-nc/4.0/   (See README for details)*/
#include "LoraSx1262.h"

byte* payload = "Hello world. This is mitch and I'm doing a Lora Test to see how this works";
LoraSx1262* radio;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Booted");

  radio = new LoraSx1262();
}

void loop() {
  // put your main code here, to run repeatedly:

  Serial.println("Loop");
  radio->transmit(payload,strlen(payload));
}
