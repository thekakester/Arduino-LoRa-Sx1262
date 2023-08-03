/*License: CC 4.0 - Attribution, NonCommercial (by Mitch Davis, github.com/thekakester)
* https://creativecommons.org/licenses/by-nc/4.0/   (See README for details)*/
#include <LoraSx1262.h>

byte* payload = "Hello world! This is a medium sized transmission, which is long enough to see timing differences between presets";
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
  * Presets give some flexibility to make the radio work how you want without needing to
  * understand the underlying concepts.  See AdvancedRadioConfig example for more advanced config.
  * Generally speaking, longer range means slower speeds, and shorter ranges allow faster speeds
  *
  * ALL TRANSMITTERS/RECEIVERS MUST HAVE MATCHING CONFIGS, otherwise
  * they can't communicate with eachother
  */

  //FREQUENCY - Set frequency to 902Mhz (default 915Mhz)
  radio.configSetFrequency(902000000);  //Freq in Hz. Must comply with your local radio regulations

  //Configuration presets. Comment/uncomment to observe how long each packet takes to transmit
  radio.configSetPreset(PRESET_DEFAULT);      //Default   - Medium range, medium speed
  //radio.configSetPreset(PRESET_FAST);       //Fast      - Faster, but less reliable at longer distances.  Use when you need fast speed and radios are closer.
  //radio.configSetPreset(PRESET_LONGRANGE);  //LongRange - Slower and more reliable.  Good for long distance or when reliability is more important than speed
}

int loopCount = 0;

void loop() {
  
  //Time how long a transmission takes
  uint32_t startTime = millis();
  radio.transmit(payload,strlen(payload));
  uint32_t elapsed = millis() - startTime;

  Serial.print("Transmission time (ms): ");
  Serial.println(elapsed);

  delay(1000);
}
