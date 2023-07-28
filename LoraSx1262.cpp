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

  configureRadioEssentials();
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

/*Send the bare-bones required commands needed for radio to run.
* Do not set custom or optional commands here, please keep this section as simplified as possible.
* Essential commands are found by reading the datasheet
*/
void LoraSx1262::configureRadioEssentials() {
  uint8_t spiBuff[9];  //A buffer for sending/receiving SPI data

  //Tell DIO2 to control the RF switch so we don't have to do it manually
  digitalWrite(SX1262_NSS,0); //Enable radio chip-select
  spiBuff[0] = 0x9D;  //Opcode for "SetDIO2AsRfSwitchCtrl"
  spiBuff[1] = 0x01;  //Enable
  SPI.transfer(spiBuff,2);
  digitalWrite(SX1262_NSS,1); //Disable radio chip-select
  delay(100); //Give time for the radio to proces command

  //Set PLL frequency (this is a complicated math equation.  See datasheet entry for SetRfFrequency)
  uint32_t pllFrequency = 959447040;  //915MHz, see Datasheet for calcualting this number
  digitalWrite(SX1262_NSS,0); //Enable radio chip-select
  spiBuff[0] = 0x86;  //Opcode for set RF Frequencty
  spiBuff[1] = (pllFrequency >> 24) & 0xFF;  //MSB of pll frequency
  spiBuff[2] = (pllFrequency >> 16) & 0xFF;  //
  spiBuff[3] = (pllFrequency >>  8) & 0xFF;  //
  spiBuff[4] = (pllFrequency >>  0) & 0xFF;  //LSB of requency
  SPI.transfer(spiBuff,5);
  digitalWrite(SX1262_NSS,1); //Disable radio chip-select
  delay(100);                  //Give time for the radio to proces command

  //Set modem to LoRa (described in datasheet section 13.4.2)
  digitalWrite(SX1262_NSS,0); //Enable radio chip-select
  spiBuff[0] = 0x8A;          //Opcode for "SetPacketType"
  spiBuff[1] = 0x01;          //Packet Type: 0x00=GFSK, 0x01=LoRa
  SPI.transfer(spiBuff,2);
  digitalWrite(SX1262_NSS,1); //Disable radio chip-select
  delay(100);                  //Give time for radio to process the command

  //Set Rx Timeout to reset on SyncWord or Header detection
  digitalWrite(SX1262_NSS,0); //Enable radio chip-select
  spiBuff[0] = 0x9F;          //Opcode for "StopTimerOnPreamble"
  spiBuff[1] = 0x00;          //Stop timer on:  0x00=SyncWord or header detection, 0x01=preamble detection  SPI.transfer(spiBuff,2);
  digitalWrite(SX1262_NSS,1); //Disable radio chip-select
  delay(100);                  //Give time for radio to process the command

  /*Set modulation parameters
  # Modulation parameters are:
  #  - SpreadingFactor
  #  - Bandwidth
  #  - CodingRate
  #  - LowDataRateOptimize
  # None of these actually matter that much.  You can set them to anything, and data will still show up
  # on a radio frequency monitor.
  # You just MUST call "setModulationParameters", otherwise the radio won't work at all*/
  digitalWrite(SX1262_NSS,0); //Enable radio chip-select
  spiBuff[0] = 0x8B;          //Opcode for "SetModulationParameters"
  spiBuff[1] = 0x07;          //ModParam1 = Spreading Factor.  Can be SF5-SF12, written in hex (0x05-0x0C)
  spiBuff[2] = 0x06;          //ModParam2 = Bandwidth.  See Datasheet 13.4.5.2 for details. 0x00=7.81khz (slowest)
  spiBuff[3] = 0x01;          //ModParam3 = CodingRate.  Semtech recommends CR_4_5 (which is 0x01).  Options are 0x01-0x04, which correspond to coding rate 5-8 respectively
  spiBuff[4] = 0x00;          //LowDataRateOptimize.  0x00 = 0ff, 0x01 = On.  Required to be on for SF11 + SF12
  SPI.transfer(spiBuff,5);
  digitalWrite(SX1262_NSS,1); //Disable radio chip-select
  delay(100);                  //Give time for radio to process the command

  // Set PA Config
  // See datasheet 13.1.4 for descriptions and optimal settings recommendations
  digitalWrite(SX1262_NSS,0); //Enable radio chip-select
  spiBuff[0] = 0x95;          //Opcode for "SetPaConfig"
  spiBuff[1] = 0x04;          //paDutyCycle. See datasheet, set in conjuntion with hpMax
  spiBuff[2] = 0x07;          //hpMax.  Basically Tx power.  0x00-0x07 where 0x07 is max power
  spiBuff[3] = 0x00;          //device select: 0x00 = SX1262, 0x01 = SX1261
  spiBuff[4] = 0x01;          //paLut (reserved, always set to 1)
  SPI.transfer(spiBuff,5);
  digitalWrite(SX1262_NSS,1); //Disable radio chip-select
  delay(100);                  //Give time for radio to process the command

  // Set TX Params
  // See datasheet 13.4.4 for details
  digitalWrite(SX1262_NSS,0); //Enable radio chip-select
  spiBuff[0] = 0x8E;          //Opcode for SetTxParams
  spiBuff[1] = 22;            //Power.  Can be -17(0xEF) to +14x0E in Low Pow mode.  -9(0xF7) to 22(0x16) in high power mode
  spiBuff[2] = 0x02;          //Ramp time. Lookup table.  See table 13-41. 0x02="40uS"
  SPI.transfer(spiBuff,3);
  digitalWrite(SX1262_NSS,1); //Disable radio chip-select
  delay(100);                  //Give time for radio to process the command

  //Set LoRa Symbol Number timeout
  //How many symbols are needed for a good receive.
  //Symbols are preamble symbols
  digitalWrite(SX1262_NSS,0); //Enable radio chip-select
  spiBuff[0] = 0xA0;          //Opcode for "SetLoRaSymbNumTimeout"
  spiBuff[1] = 0x00;          //Number of symbols.  Ping-pong example from Semtech uses 5
  SPI.transfer(spiBuff,2);
  digitalWrite(SX1262_NSS,1); //Disable radio chip-select
  delay(100);                  //Give time for radio to process the command

  //Enable interrupts
  digitalWrite(SX1262_NSS,0); //Enable radio chip-select
  spiBuff[0] = 0x08;        //0x08 is the opcode for "SetDioIrqParams"
  spiBuff[1] = 0x00;        //IRQMask MSB.  IRQMask is "what interrupts are enabled"
  spiBuff[2] = 0x02;        //IRQMask LSB         See datasheet table 13-29 for details
  spiBuff[3] = 0xFF;        //DIO1 mask MSB.  Of the interrupts detected, which should be triggered on DIO1 pin
  spiBuff[4] = 0xFF;        //DIO1 Mask LSB
  spiBuff[5] = 0x00;        //DIO2 Mask MSB
  spiBuff[6] = 0x00;        //DIO2 Mask LSB
  spiBuff[7] = 0x00;        //DIO3 Mask MSB
  spiBuff[8] = 0x00;        //DIO3 Mask LSB
  SPI.transfer(spiBuff,9);
  digitalWrite(SX1262_NSS,1); //Disable radio chip-select
  delay(100);                  //Give time for radio to process the command
}


void LoraSx1262::transmit(byte *data, int dataLen) {
  uint8_t spiBuff[32];   //Buffer for sending SPI commands to radio

  //Max lora packet size is 255 bytes
  if (dataLen > 255) { dataLen = 255;}

  /*Set packet parameters
  # Tell LoRa what kind of packet we're sending (and how long)
  # Parameters are:
  # - Preamble Length MSB
  # - Preamble Length LSB
  # - Header Type (variable vs fixed len)
  # - Payload Length
  # - CRC on/off
  # - IQ Inversion on/off
  */
  digitalWrite(SX1262_NSS,0); //Enable radio chip-select
  spiBuff[0] = 0x8C;          //Opcode for "SetPacketParameters"
  spiBuff[1] = 0x00;          //PacketParam1 = Preamble Len MSB
  spiBuff[2] = 0x0C;          //PacketParam2 = Preamble Len LSB
  spiBuff[3] = 0x00;          //PacketParam3 = Header Type. 0x00 = Variable Len, 0x01 = Fixed Length
  spiBuff[4] = dataLen;       //PacketParam4 = Payload Length (Max is 255 bytes)
  spiBuff[5] = 0x00;          //PacketParam5 = CRC Type. 0x00 = Off, 0x01 = on
  spiBuff[6] = 0x00;          //PacketParam6 = Invert IQ.  0x00 = Standard, 0x01 = Inverted
  SPI.transfer(spiBuff,7);
  digitalWrite(SX1262_NSS,1); //Disable radio chip-select
  waitForRadioCommandCompletion(100);  //Give time for radio to process the command

  //Write the payload to the buffer
  //  Reminder: PayloadLength is defined in setPacketParams
  digitalWrite(SX1262_NSS,0); //Enable radio chip-select
  spiBuff[0] = 0x0E,          //Opcode for WriteBuffer command
  spiBuff[1] = 0x00;          //Dummy byte before writing payload
  SPI.transfer(spiBuff,2);    //Send header info

  //SPI.transfer overwrites original buffer.  This could probably be confusing to the user
  //If they tried writing the same buffer twice and got different results
  //Eg "radio.transmit(buff,10); radio.transmit(buff,10);" would transmit two different packets
  //We'll make a performance+memory compromise and write in 32 byte chunks to avoid changing the contents
  //of the original data array
  //Copy contents to SPI buff until it's full, and then write that
  //TEST: I tested this method, which uses about 0.1ms (100 microseconds) more time, but it saves us about 10% of ram.
  //I think this is a fair trade 
  uint8_t size = sizeof(spiBuff);
  for (uint16_t i = 0; i < dataLen; i += size) {
    if (i + size > dataLen) { size = dataLen - i; }
    memcpy(spiBuff,&(data[i]),size);
    SPI.transfer(spiBuff,size); //Write the payload itself
  }

  digitalWrite(SX1262_NSS,1); //Disable radio chip-select
  waitForRadioCommandCompletion(1000);   //Give time for radio to process the command

  //Transmit!
  // An interrupt will be triggered if we surpass our timeout
  digitalWrite(SX1262_NSS,0); //Enable radio chip-select
  spiBuff[0] = 0x83;          //Opcode for SetTx command
  spiBuff[1] = 0xFF;          //Timeout (3-byte number)
  spiBuff[2] = 0xFF;          //Timeout (3-byte number)
  spiBuff[3] = 0xFF;          //Timeout (3-byte number)
  SPI.transfer(spiBuff,4);
  digitalWrite(SX1262_NSS,1); //Disable radio chip-select
  waitForRadioCommandCompletion(3000);
}

/**This command will wait until the radio reports that it is no longer busy.
This is useful when waiting for commands to finish that take a while such as transmitting packets.
Specify a timeout (in milliseconds) to avoid an infinite loop if something happens to the radio

Returns TRUE on success, FALSE if timeout hit
*/
bool LoraSx1262::waitForRadioCommandCompletion(uint32_t timeout) {
  uint32_t startTime = millis();
  bool dataTransmitted = false;

  uint8_t spiBuff[2];

  //Keep checking radio status until it has completed
  while (!dataTransmitted) {
    //Ask the radio for a status update
    digitalWrite(SX1262_NSS,0); //Enable radio chip-select
    spiBuff[0] = 0xC0;          //Opcode for "getStatus" command
    spiBuff[1] = 0x00;          //Dummy byte, status will overwrite this byte
    SPI.transfer(spiBuff,2);
    digitalWrite(SX1262_NSS,1); //Disable radio chip-select

    //Parse out the status (see datasheet for what each bit means)
    uint8_t chipMode = (spiBuff[1] >> 4) & 0x7;     //Chip mode is bits [6:4] (3-bits)
    uint8_t commandStatus = (spiBuff[1] >> 1) & 0x7;//Command status is bits [3:1] (3-bits)

    //Status of 0, 1, and 2 means that we're still busy.  Anything else means we're done
    //with the command
    if (commandStatus != 0 && commandStatus != 1 && commandStatus != 2) {
      dataTransmitted = true;
    }

    //If we're in standby mode, we don't need to wait at all
    //0x03 = STBY_XOSC, 0x02= STBY_RC
    if (chipMode == 0x03 || chipMode == 0x02) {
      dataTransmitted = true;
    }

    //Avoid infinite loop by implementing a timeout
    if (millis() - startTime >= timeout) {
      return false;
    }
  }

  //We did it!
  return true;
}