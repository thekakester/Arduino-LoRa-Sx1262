# Arduino-LoRa-Sx1262
A lightweight and fast library for using a LoRa 900mhz radio on an Arduino Uno (R3 and R4).
This library currently uses 2690 bytes of ProgMem and 25 bytes of RAM.

# How to use (Examples)
## Receive Example:
```C++
#include "LoraSx1262.h"

LoraSx1262* radio;
byte receiveBuff[255];

void setup() {
  Serial.begin(9600);
  Serial.println("Booted");

  radio = new LoraSx1262();
}

void loop() {
  //Receive a packet over radio
  int bytesRead = radio->lora_receive_async(receiveBuff, sizeof(receiveBuff));

  if (bytesRead > -1) {
    //Print the payload out over serial
    Serial.print("Received: ");
    Serial.write(receiveBuff,bytesRead);
    Serial.println(); //Add a newline after printing
  }
}
```

## Transmit Example
```C++
#include "LoraSx1262.h"

byte payload = "Hello world.  This a pretty long payload. We can transmit up to 255 bytes at once, which is pretty neat if you ask me";
LoraSx1262* radio;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Booted");

  radio = new LoraSx1262();
}

void loop() {
  Serial.print("Transmitting... ");
  radio->transmit(payload,strlen(payload));
  Serial.println("Done!");

  delay(1000);
}
```

# License
## Creative Commons 4.0 - Attribution, NonCommercial
https://creativecommons.org/licenses/by-nc/4.0/
See LICENSE.txt for full license
Author: Mitch Davis (2023). github.com/thekakester

You are free to:
   * Share — copy and redistribute the material in any medium or format
   * Adapt — remix, transform, and build upon the material

Under the following terms:
   * Attribution — You must give appropriate credit, provide a link to the license, and indicate if changes were made. You may do so in any reasonable manner, but not in any way that suggests the licensor endorses you or your use.
   * NonCommercial — You may not use the material for commercial purposes.

No warranties are given. The license may not give you all of the permissions necessary for your intended use.
For example, other rights such as publicity, privacy, or moral rights may limit how you use the material
