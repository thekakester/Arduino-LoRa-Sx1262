/*License: Creative Commons 4.0 - Attribution, NonCommercial
* https://creativecommons.org/licenses/by-nc/4.0/
* Author: Mitch Davis (2023). github.com/thekakester
* 
* You are free to:
*    Share — copy and redistribute the material in any medium or format
*    Adapt — remix, transform, and build upon the material
* Under the following terms:
*    Attribution — You must give appropriate credit, provide a link to the license, and indicate if changes were made.
*                  You may do so in any reasonable manner, but not in any way that suggests the licensor endorses you or your use.
*    NonCommercial — You may not use the material for commercial purposes.
*
* No warranties are given. The license may not give you all of the permissions necessary for your intended use.
* For example, other rights such as publicity, privacy, or moral rights may limit how you use the material
*/

#ifndef __LORA1262__
#define __LORA1262__

#include <Arduino.h>
#include <SPI.h>

/* Wiring requirements (for default shield pinout)
# +-----------------+-------------+------------+
# | Description     | Arduino Pin | Sx1262 Pin |
# +-----------------+-------------+------------+
# | Power (3v3)     | 3v3         | 3v3        |
# | GND             | GND         | GND        |
# | Radio Reset     | A0          | SX_NRESET  |
# | Busy (optional) | D3          | BUSY       |
# | Radio Interrupt | D5          | DIO1       |
# | SPI SCK         | 13          | SCK        |
# | SPI MOSI        | D11         | MOSI       |
# | SPI MISO        | D12         | MISO       |
# | SPI CS          | D7          | NSS        |
# +-----------------+----------+------------+
*/

//Pin configurations (for Arduino UNO)
#define SX1262_NSS   7
#define SX1262_RESET A0
#define SX1262_DIO1  5

//Presets. These help make radio config easier
#define PRESET_DEFAULT    0
#define PRESET_LONGRANGE  1
#define PRESET_FAST       2

class LoraSx1262 {
  public:
    bool begin();
    bool sanityCheck(); /*Returns true if we have an active SPI communication with the radio*/
    void transmit(byte* data, int dataLen);
    int lora_receive_async(byte* buff, int buffMaxLen); /*Checks to see if a lora packet was received yet, returns the packet if available*/
    int lora_receive_blocking(byte* buff, int buffMaxLen, uint32_t timeout); /*Waits until a packet is received, with an optional timeout*/

    //Radio configuration (optional)
    bool configSetPreset(int preset);
    bool configSetFrequency(long frequencyInHz);
    bool configSetBandwidth(int bandwidth);
    bool configSetCodingRate(int codingRate);
    bool configSetSpreadingFactor(int spreadingFactor);
    
    //These variables show signal quality, and are updated automatically whenever a packet is received
    int rssi = 0;
    int snr = 0;
    int signalRssi = 0;

    uint32_t frequencyToPLL(long freqInHz);

  private:
    void setModeReceive();  //Puts the radio in receive mode, allowing it to receive packets
    void setModeStandby();  //Put radio into standby mode.  Switching from Rx to Tx directly is slow
    void configureRadioEssentials();
    bool waitForRadioCommandCompletion(uint32_t timeout);
    void updateRadioFrequency();
    void updateModulationParameters();
    bool inReceiveMode = false;
    uint8_t spiBuff[32];   //Buffer for sending SPI commands to radio

    //Config variables (set to PRESET_DEFAULT on init)
    uint32_t pllFrequency;
    uint8_t bandwidth;
    uint8_t codingRate;
    uint8_t spreadingFactor;
    uint8_t lowDataRateOptimize;
    uint32_t transmitTimeout; //Worst-case transmit time depends on some factors
};

#endif