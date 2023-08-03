#include "Arduino.h"
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

bool LoraSx1262::begin() {
  //Set up SPI to talk to the LoRa Radio shield
  SPI.begin();

  //Set I/O pins based on the configuration
  //See .h file for pinout diagram
  pinMode(SX1262_NSS,OUTPUT);
  digitalWrite(SX1262_NSS, 1);  //High = inactive

  pinMode(SX1262_RESET,OUTPUT);
  digitalWrite(SX1262_RESET, 1);  //High = inactive

  pinMode(SX1262_DIO1, INPUT);  //Radio interrupt pin.  Goes high when we receive a packet

  //Hardware reset the radio by toggling the reset pin
  digitalWrite(SX1262_RESET, 0); delay(10);
  digitalWrite(SX1262_RESET, 1); delay(10);
  
  //Ensure SPI communication is working with the radio
  bool success = sanityCheck();
  if (!success) { return false; }

  //Run the bare-minimum required SPI commands to set up the radio to use
  this->configureRadioEssentials();
  
  return true;  //Return success that we set up the radio
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
  //Tell DIO2 to control the RF switch so we don't have to do it manually
  digitalWrite(SX1262_NSS,0); //Enable radio chip-select
  spiBuff[0] = 0x9D;  //Opcode for "SetDIO2AsRfSwitchCtrl"
  spiBuff[1] = 0x01;  //Enable
  SPI.transfer(spiBuff,2);
  digitalWrite(SX1262_NSS,1); //Disable radio chip-select
  delay(100); //Give time for the radio to proces command

  //Just a single SPI command to set the frequency, but it's broken out
  //into its own function so we can call it on-the-fly when the config changes
  this->configSetFrequency(915000000);  //Set default frequency to 915mhz

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

  //Set modulation parameters is just one more SPI command, but since it
  //is often called frequently when changing the radio config, it's broken up into its own function
  this->configSetPreset(PRESET_DEFAULT);  //Sets default modulation parameters

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
  waitForRadioCommandCompletion(this->transmitTimeout); //Wait for tx to complete, with a timeout so we don't wait forever

  //Remember that we are in Tx mode.  If we want to receive a packet, we need to switch into receiving mode
  inReceiveMode = false;
}

/**This command will wait until the radio reports that it is no longer busy.
This is useful when waiting for commands to finish that take a while such as transmitting packets.
Specify a timeout (in milliseconds) to avoid an infinite loop if something happens to the radio

Returns TRUE on success, FALSE if timeout hit
*/
bool LoraSx1262::waitForRadioCommandCompletion(uint32_t timeout) {
  uint32_t startTime = millis();
  bool dataTransmitted = false;

  //Keep checking radio status until it has completed
  while (!dataTransmitted) {
    //Wait some time between spamming SPI status commands, asking if the chip is ready yet
    //Some commands take a bit before the radio even changes into a busy state,
    //so if we check too fast we might pre-maturely think we're done processing the command
    //3ms delay gives inconsistent results.  4ms seems stable.  Using 5ms to be safe
    delay(5);

    //Ask the radio for a status update
    digitalWrite(SX1262_NSS,0); //Enable radio chip-select
    spiBuff[0] = 0xC0;          //Opcode for "getStatus" command
    spiBuff[1] = 0x00;          //Dummy byte, status will overwrite this byte
    SPI.transfer(spiBuff,2);
    digitalWrite(SX1262_NSS,1); //Disable radio chip-select

    //Parse out the status (see datasheet for what each bit means)
    uint8_t chipMode = (spiBuff[1] >> 4) & 0x7;     //Chip mode is bits [6:4] (3-bits)
    uint8_t commandStatus = (spiBuff[1] >> 1) & 0x7;//Command status is bits [3:1] (3-bits)

    //Status 0, 1, 2 mean we're still busy.  Anything else means we're done.
    //Commands 3-6 = command timeout, command processing error, failure to execute command, and Tx Done (respoectively)
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

//Sets the radio into receive mode, allowing it to listen for incoming packets.
//If radio is already in receive mode, this does nothing.
//There's no such thing as "setModeTransmit" because it is set automatically when transmit() is called
void LoraSx1262::setModeReceive() {
  if (inReceiveMode) { return; }  //We're already in receive mode, this would do nothing

  //Set packet parameters
  digitalWrite(SX1262_NSS,0); //Enable radio chip-select
  spiBuff[0] = 0x8C;          //Opcode for "SetPacketParameters"
  spiBuff[1] = 0x00;          //PacketParam1 = Preamble Len MSB
  spiBuff[2] = 0x0C;          //PacketParam2 = Preamble Len LSB
  spiBuff[3] = 0x00;          //PacketParam3 = Header Type. 0x00 = Variable Len, 0x01 = Fixed Length
  spiBuff[4] = 0xFF;          //PacketParam4 = Payload Length (Max is 255 bytes)
  spiBuff[5] = 0x00;          //PacketParam5 = CRC Type. 0x00 = Off, 0x01 = on
  spiBuff[6] = 0x00;          //PacketParam6 = Invert IQ.  0x00 = Standard, 0x01 = Inverted
  SPI.transfer(spiBuff,7);
  digitalWrite(SX1262_NSS,1); //Disable radio chip-select
  waitForRadioCommandCompletion(100);

  // Tell the chip to wait for it to receive a packet.
  // Based on our previous config, this should throw an interrupt when we get a packet
  digitalWrite(SX1262_NSS,0); //Enable radio chip-select
  spiBuff[0] = 0x82;          //0x82 is the opcode for "SetRX"
  spiBuff[1] = 0xFF;          //24-bit timeout, 0xFFFFFF means no timeout
  spiBuff[2] = 0xFF;          // ^^
  spiBuff[3] = 0xFF;          // ^^
  SPI.transfer(spiBuff,4);
  digitalWrite(SX1262_NSS,1); //Disable radio chip-select
  waitForRadioCommandCompletion(100);

  //Remember that we're in receive mode so we don't need to run this code again unnecessarily
  inReceiveMode = true;
}

/*Receive a packet if available
If available, this will return the size of the packet and store the packet contents into the user-provided buffer.
A max length of the buffer can be provided to avoid buffer overflow.  If buffer is not large enough for entire payload, overflow is thrown out.
Recommended to pass in a buffer that is 255 bytes long to make sure you can received any lora packet that comes in.

Returns -1 when no packet is available.
Returns 0 when an empty packet is received (packet with no payload)
Returns payload size (1-255) when a packet with a non-zero payload is received. If packet received is larger than the buffer provided, this will return buffMaxLen
*/
int LoraSx1262::lora_receive_async(byte* buff, int buffMaxLen) {
  setModeReceive(); //Sets the mode to receive (if not already in receive mode)

  //Radio pin DIO1 (interrupt) goes high when we have a packet ready.  If it's low, there's no packet yet
  if (digitalRead(SX1262_DIO1) == false) { return -1; } //Return -1, meanining no packet ready

  //Tell the radio to clear the interrupt, and set the pin back inactive.
  while (digitalRead(SX1262_DIO1)) {
    //Clear all interrupt flags.  This should result in the interrupt pin going low
    digitalWrite(SX1262_NSS,0); //Enable radio chip-select
    spiBuff[0] = 0x02;          //Opcode for ClearIRQStatus command
    spiBuff[1] = 0xFF;          //IRQ bits to clear (MSB) (0xFFFF means clear all interrupts)
    spiBuff[2] = 0xFF;          //IRQ bits to clear (LSB)
    SPI.transfer(spiBuff,3);
    digitalWrite(SX1262_NSS,1); //Disable radio chip-select
  }

  // (Optional) Read the packet status info from the radio.
  // This is things like radio strength, noise, etc.
  // See datasheet 13.5.3 for more info
  // This provides debug info about the packet we received
  digitalWrite(SX1262_NSS,0); //Enable radio chip-select
  spiBuff[0] = 0x14;          //Opcode for get packet status
  spiBuff[1] = 0xFF;          //Dummy byte. Returns status
  spiBuff[2] = 0xFF;          //Dummy byte. Returns rssi
  spiBuff[3] = 0xFF;          //Dummy byte. Returns snd
  spiBuff[4] = 0xFF;          //Dummy byte. Returns signal RSSI
  SPI.transfer(spiBuff,5);
  digitalWrite(SX1262_NSS,1); //Disable radio chip-select

  //Store these values as class variables so they can be accessed if needed
  //Documentation for what these variables mean can be found in the .h file
  rssi       = -((int)spiBuff[2]) / 2;  //"Average over last packet received of RSSI. Actual signal power is –RssiPkt/2 (dBm)"
  snr        =  ((int8_t)spiBuff[3]) / 4;   //SNR is returned as a SIGNED byte, so we need to do some conversion first
  signalRssi = -((int)spiBuff[4]) / 2;
    
  //We're almost ready to read the packet from the radio
  //But first we have to know how big the packet is, and where in the radio memory it is stored
  digitalWrite(SX1262_NSS,0); //Enable radio chip-select
  spiBuff[0] = 0x13;          //Opcode for GetRxBufferStatus command
  spiBuff[1] = 0xFF;          //Dummy.  Returns radio status
  spiBuff[2] = 0xFF;          //Dummy.  Returns loraPacketLength
  spiBuff[3] = 0xFF;          //Dummy.  Returns memory offset (address)
  SPI.transfer(spiBuff,4);
  digitalWrite(SX1262_NSS,1); //Disable radio chip-select

  uint8_t payloadLen = spiBuff[2];    //How long the lora packet is
  uint8_t startAddress = spiBuff[3];  //Where in 1262 memory is the packet stored

  //Make sure we don't overflow the buffer if the packet is larger than our buffer
  if (buffMaxLen < payloadLen) {payloadLen = buffMaxLen;}

  //Read the radio buffer from the SX1262 into the user-supplied buffer
  digitalWrite(SX1262_NSS,0); //Enable radio chip-select
  spiBuff[0] = 0x1E;          //Opcode for ReadBuffer command
  spiBuff[1] = startAddress;  //SX1262 memory location to start reading from
  spiBuff[2] = 0x00;          //Dummy byte
  SPI.transfer(spiBuff,3);    //Send commands to get read started
  SPI.transfer(buff,payloadLen);  //Get the contents from the radio and store it into the user provided buffer
  digitalWrite(SX1262_NSS,1); //Disable radio chip-select

  return payloadLen;  //Return how many bytes we actually read
}

/*Waits for a packet to come in.  This code will block until something is received, or the timeout is hit.
Set timeout=0 for no timeout, or set to a positive integer to specify a timeout in milliseconds
This will store the contents of the payload into the user-provided buffer.  Recommended to use a buffer with 255 bytes to always receive full packet.
If a smaller buffer is used, maxBuffLen can be set to avoid buffer overflow. Any packets larger than this will have remaining bytes thrown out.

Returns -1 when no packet is available and timeout was hit.
Returns 0 when an empty packet is received (packet with no payload)
Returns payload size (1-255) when a packet with a non-zero payload is received. If packet received is larger than the buffer provided, this will return buffMaxLen
*/
int LoraSx1262::lora_receive_blocking(byte *buff, int buffMaxLen, uint32_t timeout) {
  setModeReceive(); //Sets the mode to receive (if not already in receive mode)

  uint32_t startTime = millis();
  uint32_t elapsed = startTime;

  //Wait for radio interrupt pin to go high, indicating a packet was received, or if we hit our timeout
  while (digitalRead(SX1262_DIO1) == false) {
    //If user specified a timeout, check if we hit it
    if (timeout > 0) {
      elapsed = millis() - startTime;
      if (elapsed >= timeout) {
        return -1;    //Return error, saying that we hit our timeout
      }
    }
  }

  //If our pin went high, then we got a packet!  Return it
  return lora_receive_async(buff,buffMaxLen);
}

//Set the radio frequency.  Just a single SPI call,
//but this is broken out to make it more convenient to change frequency on-the-fly
//You must set this->pllFrequency before calling this
void LoraSx1262::updateRadioFrequency() {
  //Set PLL frequency (this is a complicated math equation.  See datasheet entry for SetRfFrequency)
  digitalWrite(SX1262_NSS,0); //Enable radio chip-select
  spiBuff[0] = 0x86;  //Opcode for set RF Frequencty
  spiBuff[1] = (this->pllFrequency >> 24) & 0xFF;  //MSB of pll frequency
  spiBuff[2] = (this->pllFrequency >> 16) & 0xFF;  //
  spiBuff[3] = (this->pllFrequency >>  8) & 0xFF;  //
  spiBuff[4] = (this->pllFrequency >>  0) & 0xFF;  //LSB of requency
  SPI.transfer(spiBuff,5);
  digitalWrite(SX1262_NSS,1); //Disable radio chip-select
  delay(100);                  //Give time for the radio to proces command
}

//Set the radio modulation parameters.
//This is things like bandwitdh, spreading factor, coding rate, etc.
//This is broken into its own function because this command might get called frequently
void LoraSx1262::updateModulationParameters() {
  /*Set modulation parameters
  # Modulation parameters are:
  #  - SpreadingFactor
  #  - Bandwidth
  #  - CodingRate
  #  - LowDataRateOptimize
  # None of these actually matter that much.  You can set them to anything, and data will still show up
  # on a radio frequency monitor.
  # You just MUST call "setModulationParameters", otherwise the radio won't work at all*/
  digitalWrite(SX1262_NSS,0);       //Enable radio chip-select
  spiBuff[0] = 0x8B;                //Opcode for "SetModulationParameters"
  spiBuff[1] = this->spreadingFactor;     //ModParam1 = Spreading Factor.  Can be SF5-SF12, written in hex (0x05-0x0C)
  spiBuff[2] = this->bandwidth;           //ModParam2 = Bandwidth.  See Datasheet 13.4.5.2 for details. 0x00=7.81khz (slowest)
  spiBuff[3] = this->codingRate;          //ModParam3 = CodingRate.  Semtech recommends CR_4_5 (which is 0x01).  Options are 0x01-0x04, which correspond to coding rate 5-8 respectively
  spiBuff[4] = this->lowDataRateOptimize; //LowDataRateOptimize.  0x00 = 0ff, 0x01 = On.  Required to be on for SF11 + SF12
  SPI.transfer(spiBuff,5);
  digitalWrite(SX1262_NSS,1); //Disable radio chip-select
  delay(100);                  //Give time for radio to process the command

  //Come up with a reasonable timeout for transmissions
  //SF12 is painfully slow, so we want a nice long timeout for that,
  //but we really don't want someone using SF5 to have to wait MINUTES for a timeout
  //I came up with these timeouts by measuring how long it actually took to transmit a packet
  //at each spreading factor with a MAX 255-byte payload and 7khz Bandwitdh (the slowest one)
  switch (this->spreadingFactor) {
    case 12:
      this->transmitTimeout = 252000; //Actual tx time 126 seconds
      break;
    case 11:
      this->transmitTimeout = 160000; //Actual tx time 81 seconds
      break;
    case 10:
      this->transmitTimeout = 60000; //Actual tx time 36 seconds
      break;
    case 9:
      this->transmitTimeout = 40000; //Actual tx time 20 seconds
      break;
    case 8:
      this->transmitTimeout = 20000; //Actual tx time 11 seconds
      break;
    case 7:
      this->transmitTimeout = 12000; //Actual tx time 6.3 seconds
      break;
    case 6:
      this->transmitTimeout = 7000; //Actual tx time 3.7s seconds
      break;
    default:  //SF5
      this->transmitTimeout = 5000; //Actual tx time 2.2 seconds
      break;
  }
}


//--------------------------
// ADVANCED FUNCTIONS
//--------------------------
//The functions below are intended for advanced users who are more familiar with LoRa Radios at a lower level


/**(Optional) Use one of the pre-made radio configurations
* This is ideal for making simple changes to the radio config
* without needing to understand how the underlying settings work
* 
* Argument: pass in one of the following
*     - PRESET_DEFAULT:   Default radio config.
*                         Medium range, medium speed
*     - PRESET_FAST:      Faster speeds, but less reliable at long ranges.
*                         Use when you need fast data transfer and have radios close together
*     - PRESET_LONGRANGE: Most reliable option, but slow. Suitable when you prioritize
*                         reliability over speed, or when transmitting over long distances
*/
bool LoraSx1262::configSetPreset(int preset) {
  if (preset == PRESET_DEFAULT) {
    this->bandwidth = 5;            //250khz
    this->codingRate = 1;           //CR_4_5
    this->spreadingFactor = 7;      //SF7
    this->lowDataRateOptimize = 0;  //Don't optimize (used for SF12 only)
    this->updateModulationParameters();
    return true;
  }

  if (preset == PRESET_LONGRANGE) {
    this->bandwidth = 4;            //125khz
    this->codingRate = 1;           //CR_4_5
    this->spreadingFactor = 12;     //SF12
    this->lowDataRateOptimize = 1;  //Optimize for low data rate (SF12 only)
    this->updateModulationParameters();
    return true;
  }

  if (preset == PRESET_FAST) {
    this->bandwidth = 6;            //500khz
    this->codingRate = 1;           //CR_4_5
    this->spreadingFactor = 5;      //SF5
    this->lowDataRateOptimize = 0;  //Don't optimize (used for SF12 only)
    this->updateModulationParameters();
    return true;
  }

  //Invalid preset specified
  return false;
}

/** (Optional) Set the operating frequency of the radio.
* The 1262 radio supports 150-960Mhz.  This library uses a default of 915Mhz.
* MAKE SURE THAT YOU ARE OPERATING IN A FREQUENCY THAT IS ALLOWED IN YOUR COUNTRY!
* For example, 915mhz (915000000 hz) is safe in the US.
*
* Specify the desired frequency in Hz (eg 915MHZ is 915000000).
* Returns TRUE on success, FALSE on invalid frequency
*/
bool LoraSx1262::configSetFrequency(long frequencyInHz) {
  //Make sure the specified frequency is in the valid range.
  if (frequencyInHz < 150000000 || frequencyInHz > 960000000) { return false;}

  //Calculate the PLL frequency (See datasheet section 13.4.1 for calculation)
  //PLL frequency controls the radio's clock multipler to achieve the desired frequency
  this->pllFrequency = frequencyToPLL(frequencyInHz);
  updateRadioFrequency();
}

/*Set the bandwith (basically, this is how big the frequency span is that we occupy)
* Bigger bandwidth allows us to transmit large amounts of data faster, but it occupies a larger span of frequencies.
* Smaller bandwith takes longer to transmit large amounts of data, but its less likely to collide with other frequencies.
*
* Available bandwidth settings, pulled from datasheet 13.4.5.2
*  SETTING.   | Bandwidth
* ------------+-----------
*    0x00     |    7.81khz
*    0x08     |   10.42khz
*    0x01     |   15.63khz
*    0x09     |   20.83khz
*    0x02     |   31.25khz
*    0x0A     |   41.67khz
*    0x03     |   62.50khz
*    0x04     |  125.00khz
*    0x05     |  250.00khz
*    0x06     |  500.00khz (default)
*
* Returns TRUE on success, FALSE on failure (invalid bandwidth)
*/
bool LoraSx1262::configSetBandwidth(int bandwidth) {
  //Bandwidth setting must be 0-10 (excluding 7 for some reason)
  if (bandwidth < 0 || bandwidth > 0x0A || bandwidth == 7) { return false; }
  this->bandwidth = bandwidth;
  this->updateModulationParameters();
  return true;
}

/*I honestly don't really know what coding rate means.  It's something technical to have to do with radios
* Set it here if you want.  See datasheet 13.4.5.2 for details
*  SETTING. | Coding Rate
* ----------+--------------------
*    0x01   |   CR_4_5 (default)
*    0x02   |   CR_4_6
*    0x03   |   CR_4_7
*    0x04   |   CR_4_8
*
* Returns TRUE on success, FALSE on failure (invalid coding rate)
*/
bool LoraSx1262::configSetCodingRate(int codingRate) {
  //Coding rate must be 1-4 (inclusive)
  if (codingRate < 1 || codingRate > 4) { return false; }
  this->codingRate = codingRate;
  this->updateModulationParameters();
}

/*Change the spreading factor of a packet
The higher the spreading factor, the slower and more reliable the transmission will be.
Higher spreading factors are good for longer distances with slower transmit speeds.
Lower spreading factors are good when the radios are close, which allows faster transmission speeds.

* Setting | Spreading Factor
* --------+---------------------------
*    5    | SF5 (fastest, short range)
*    6    | SF6
*    7    | SF7 (default)
*    8    | SF8
*    9    | SF9
*   10    | SF10 
*   11    | SF11
*   12    | SF12 (Slowest, long range, most reliable)
*
* Returns TRUE on success, FALSE on failure (incorrect spreading factor)
*/
bool LoraSx1262::configSetSpreadingFactor(int spreadingFactor) {
  if (spreadingFactor < 5 || spreadingFactor > 12) { return false; }

  //The datasheet highly recommends enabling "LowDataRateOptimize" for SF11 and SF12
  this->lowDataRateOptimize = (spreadingFactor >= 11) ? 1 : 0;  //Turn on for SF11+SF12, turn off for anything else
  this->spreadingFactor = spreadingFactor;
  this->updateModulationParameters();
}

/*Convert a frequency in hz (such as 915000000) to the respective PLL setting.
* The radio requires that we set the PLL, which controls the multipler on the internal clock to achieve the desired frequency.
* Valid frequencies are 150mhz to 960mhz (150000000 to 960000000)
*
* NOTE: This assumes the radio is using a 32mhz clock, which is standard.  This is independent of the microcontroller clock
* See datasheet section 13.4.1 for this calculation.
* Example: 915mhz (915000000) has a PLL of 959447040
*/
uint32_t LoraSx1262::frequencyToPLL(long rfFreq) {
  /* Datasheet Says:
	 *		rfFreq = (pllFreq * xtalFreq) / 2^25
	 * Rewrite to solve for pllFreq
	 *		pllFreq = (2^25 * rfFreq)/xtalFreq
	 *
	 *	In our case, xtalFreq is 32mhz
	 *	pllFreq = (2^25 * rfFreq) / 32000000
	 */

	//Basically, we need to do "return ((1 << 25) * rfFreq) / 32000000L"
  //It's very important to perform this without losing precision or integer overflow.
  //If arduino supported 64-bit varibales (which it doesn't), we could just do this:
  //    uint64_t firstPart = (1 << 25) * (uint64_t)rfFreq;
  //    return (uint32_t)(firstPart / 32000000L);
  //
  //Instead, we need to break this up mathimatically to avoid integer overflow
  //First, we'll simplify the equation by dividing both parts by 2048 (2^11)
  //    ((1 << 25) * rfFreq) / 32000000L      -->      (16384 * rfFreq) / 15625;
  //
  // Now, we'll divide first, then multiply (multiplying first would cause integer overflow)
  // Because we're dividing, we need to keep track of the remainder to avoid losing precision
  uint32_t q = rfFreq / 15625UL;  //Gives us the result (quotient), rounded down to the nearest integer
  uint32_t r = rfFreq % 15625UL;  //Everything that isn't divisible, aka "the part that hasn't been divided yet"

  //Multiply by 16384 to satisfy the equation above
  q *= 16384UL;
  r *= 16384UL; //Don't forget, this part still needs to be divided because it was too small to divide before
  
  return q + (r / 15625UL);  //Finally divide the the remainder part before adding it back in with the quotient
}

