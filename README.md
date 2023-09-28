# Arduino-LoRa-Sx1262
Long-Range Radio implementation for Arduino using LoRa Sx1262 900mhz radio.  Extremely lightweight and fast code that works on many Arduino platforms.

This library uses:
 - 2042 bytes of flash
 - 50 bytes of ram
 - <sup><sub>(Updated of v1.0, Aug 2023)</sub></sup>

## Compatibility

### Arduino Boards
This library should work for any Arduino compatible board, for 3.3V or 5V platforms.  Here's a list of boards I explicitly tested.

| Arduino Board                       | Compatible |  Tested (+lib ver) | Notes                                                                                  |
|-------------------------------------|------------|--------------------|----------------------------------------------------------------------------------------|
| Arduino Uno (R3, Atmel)             | Yes        | Yes, (V1.0)        |                                                                                        |
| Arduino Uno (R4, Renesas)           | Yes        | Yes, (V1.0)        |                                                                                        |
| Arduino Nano 33 IOT                 | Yes        | Yes, (V1.0)        | <sup><sub>Requires manual wiring (shield doesn't fit this form factor)</sub></sup>     |
| Arduino Nano                        | Yes        | Planned            | <sup><sub>Requires manual wiring (shield doesn't fit this form factor)</sub></sup>     |
| Arduino Zero                        | Yes        | Yes, (V1.0)        | <sup><sub>Requires manual wiring (shield doesn't work with default pinout)</sub></sup> |
| Arduino Mega                        | Yes        | Planned            |                                                                                        |
| SparkFun SAMD21 Dev (Arm M0)        | Yes        | Planned            | <sup><sub>Requires manual wiring (shield doesn't fit this form factor)</sub></sup>     |
| Adafruit Metro ESP32-S2 Express     | Yes        | Yes, (V1.0)        | <sup><sub>Requires manual wiring (shield doesn't work with default pinout)</sub></sup> |
| Adafruit METRO M0 Express           | Yes        | Yes, (V1.0)        | <sup><sub>Requires manual wiring (shield doesn't work with default pinout)</sub></sup> |
| SparkFun RedBoard Turbo (Cortex M0) | Yes        | Yes, (V1.0)        | <sup><sub>Requires manual wiring (shield doesn't work with default pinout)</sub></sup> |
| Seeeduino Cortex-M0+(Seeeduino Zero)| Yes        | Yes, (V1.0)        | <sup><sub>Requires manual wiring (shield doesn't work with default pinout)</sub></sup> |

### LoRa Modules / Shields
| LoRa Module/Shield                      | Compatible |  Tested (+lib ver) |
|-----------------------------------------|------------|--------------------|
| Douglas LoRa Sx1262 Breakout            | Yes        | Yes, (V1.0)        |
| Douglas LoRa Sx1262 Breakout            | Yes        | Yes, (V1.0)        |
| Connected Development CDDK-SX1262-01001 | Yes        | Untested           |



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
