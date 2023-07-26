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
  private:
  
};


#endif