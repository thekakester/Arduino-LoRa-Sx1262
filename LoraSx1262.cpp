#include "LoraSx1262.h"

LoraSx1262::LoraSx1262() {
  //Set up SPI to talk to the LoRa Radio shield
  SPI.begin();

  //Set I/O pins based on the configuration
  //See .h file for pinout diagram
  pinMode(SX1262_NSS,OUTPUT);
  digitalWrite(SX1262_NSS, 1);  //High = inactive

  pinMode(SX1262_RESET,OUTPUT);
  digitalWrite(SX1262_RESET, 1);  //High = inactive

  //Hardware reset the radio by toggling the reset pin
  digitalWrite(SX1262_RESET, 0); delay(10);
  digitalWrite(SX1262_RESET, 1); delay(10);
  
  //Ensure SPI communication is working with the radio
  bool success = sanityCheck();
  if (!success) { return; }
}

/* Tests that SPI is communicating correctly with the radio.
* If this fails, check your SPI wiring.  This does not require any setup to run.
* We test the radio by reading a register that should have a known value.
*
* Returns: True if radio is communicating over SPI. False if no connection.
*/
bool LoraSx1262::sanityCheck() {

  uint16_t addressToRead = 0x0740;

  digitalWrite(SX1262_NSS, 0);  //CS Low = Enabled
  SPI.transfer(0x1D); //OpCode for "read register"
  SPI.transfer16(addressToRead);
  SPI.transfer(0x00);  //Dummy byte
  uint8_t regValue = SPI.transfer(0x00);  //Read response
  digitalWrite(SX1262_NSS, 1);  //CS High = Disabled

  return regValue == 0x14;  //Success if we read 0x14 from the register
}