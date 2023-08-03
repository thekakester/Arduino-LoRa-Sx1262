/*License: CC 4.0 - Attribution, NonCommercial (by Mitch Davis, github.com/thekakester)
* https://creativecommons.org/licenses/by-nc/4.0/   (See README for details)*/
#include <LoraSx1262.h>

byte* payload = "Hello world.  This a pretty long payload. We can transmit up to 255 bytes at once, which is pretty neat if you ask me";
LoraSx1262 radio;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Booted");

  if (!radio.begin()) { //Initialize radio
    Serial.println("Failed to initialize radio.");
  }

  /************************
  * OPTIONAL CONFIGURATION
  *************************
  * This is intended for people who know what these settings mean.
  * If you don't know what these mean, you can just ignore them to use defaults instead.
  * You can also choose to use presets instead if you'd like customization without needing to understand
  * the underlying concepts.  See RadioConfigPresets example for details.
  *
  * ALL TRANSMITTERS/RECEIVERS MUST HAVE MATCHING CONFIGS, otherwise
  * they can't communicate with eachother
  */

  //FREQUENCY - Set frequency to 902Mhz (default 915Mhz)
  radio.configSetFrequency(902000000);  //Freq in Hz. Must comply with your local radio regulations

  //BANDWIDTH - Set bandwidth to 250khz (default 500khz)
  radio.configSetBandwidth(5);  //0=7.81khz, 5=200khz, 6=500khz. See documentation for more

  //CODING RATE - Set the coding rate to CR_4_6
  radio.configSetCodingRate(2); //1-4 = coding rate CR_4_5, CR_4_6, CR_4_7, and CR_4_8 respectively

  //SPREADING FACTOR - Set the spreading factor to SF12.  (default is SF7)
  radio.configSetSpreadingFactor(12); //5-12 are valid ranges.  5 is fast and short range, 12 is slow and long range
}

void loop() {
  Serial.print("Transmitting... ");
  radio.transmit(payload,strlen(payload));
  Serial.println("Done!");

  delay(1000);
}
