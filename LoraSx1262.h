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

class LoraSx1262 {
  public:
    LoraSx1262();
    bool sanityCheck(); /*Returns true if we have an active SPI communication with the radio*/
    void transmit(byte* data, int dataLen);
    void setModeReceive();  //Puts the radio in receive mode, allowing it to receive packets
  private:
    void configureRadioEssentials();
    bool waitForRadioCommandCompletion(uint32_t timeout);
};


#endif