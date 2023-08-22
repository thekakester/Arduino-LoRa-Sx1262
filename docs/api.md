# LoraSx1262 library

## Methods

* [begin()](#begin)
* [transmit()](#transmit)
* [receive_async()](#receive_async)
* [receive_blocking()](#receive_blocking)
* [configSetPreset()](#configSetPreset)
* [configSetFrequency()](#configSetFrequency)
* [configSetBandwidth()](#configSetBandwidth)
* [configSetCodingRate()](#configSetCodingRate)
* [configSetSpreadingFactor()](#configSetSpreadingFactor)


### `begin()`

Initialize Radio with bare-bones configuration required for it to work.
This uses the library default pinouts, which can be configured manually if you are not using one of the supported arduino shields.

This initilizes the radio with default radio settings, which are good for most applications.  For most applications, you will not have to change any of the defaults.  Advanced users can specify each configuration option explicitly. Beginner and Intermediate users can use configuration presets.  See [configSetPreset()](#configSetPreset) for details.

Default Radio Configuration:
* Frequency: 915mhz
* Bandwidth: 250khz
* Spreading Factor: 7
* Coding Rate: 1
* Low Data Rate Optimize: Off

#### Syntax

```C++
radio.begin() 
```

#### Parameters

None

#### Returns
* true on success
* false if radio failed to initialize 

#### Example

```C++
#include <LoraSx1262.h> 

LoraSx1262 radio;

void setup() 
{ 
  Serial.begin(9600);
  if (!radio.begin()) { //Initialize radio
    Serial.println("Failed to initialize radio.");
  }
} 

void loop() {} 
```

### `transmit()`

Transmit a lora packet. Transmitter and receiver must have the same config to be able to talk to eachother.  If you are using a radio configuration other than the defaults (which are set up in `begin()`), you must make sure they match on transmitter and receiver.

#### Syntax

```C++
radio.transmit(byte *data, int dataLen)
```

#### Parameters

* _data_: A pointer to the payload to be sent. Payload can be 0-256 bytes long.  This can be raw binary data, or a string (`char*`)
* _dataLen_: The length of `data` in bytes. This must be 0-256.

#### Example

```C++
#include <LoraSx1262.h> 

byte* payload = "Hello world.  This a pretty long payload. We can transmit up to 255 bytes at once, which is pretty neat if you ask me";
LoraSx1262 radio;

void setup() 
{ 
  Serial.begin(9600);
  if (!radio.begin()) { //Initialize radio
    Serial.println("Failed to initialize radio.");
  }
} 

void loop() {
  //Transmit a packet every 1 second
  radio.transmit(payload,strlen(payload));
  delay(1000);
} 
```

### `receive_async()`

Non-blocking receive.  If a packet has been received by the radio, this function will copy it from the radio to the user provided buffer.  If no packet has been received by the radio yet, this function will do nothing, and will not prevent the rest of your code for running.

This will receive 0-255 bytes, which is dependent on the packet size from the transmitter. If the user provided buffer cannot hold the entire packet, the overflow is discarded.

See `receive_blocking()` if you need your code to halt until a packet is received

#### Syntax

```C++
radio.receive_async(byte* buff, int buffMaxLen)
```

#### Parameters

* _buff_: A user provided byte array for the packet payload to be copied into.  Contents of this array will be overwritten by this function.
* _buffMaxLen_: The maximum length of the buffer provided above.  This prevents a buffer overflow in the event that a received packet payload is larger than the buffer provided.

#### Returns
* -1 When no packet has been received by the radio yet
* 0 When a packet with an empty payload has been received
* 1-255 when a non-empty payload has been received. This is the length of the packet payload.  This will always return the ACTUAL payload size, even if it is too large to fit in the user provided buffer.

#### Example

```C++
#include <LoraSx1262.h>

LoraSx1262 radio;
byte receiveBuff[255];

void setup() {
  Serial.begin(9600);

  if (!radio.begin()) { //Initialize the radio
    Serial.println("Failed to initialize radio");
  }
}

void loop() {
  //Receive a packet over radio
  int bytesRead = radio.lora_receive_async(receiveBuff, sizeof(receiveBuff));

  if (bytesRead > -1) {
    //Print the payload out over serial
    Serial.print("Received: ");
    Serial.write(receiveBuff,bytesRead);
    Serial.println(); //Add a newline after printing
  }
}
```

#### See also

* [receive_blocking()](#receive_blocking)


### `receive_blocking()`

Blocking receive.  If a packet has been received by the radio, this function will copy it from the radio to the user provided buffer.  If no packet has been received by the radio yet, this function wait until a packet is received.

This will receive 0-255 bytes, which is dependent on the packet size from the transmitter. If the user provided buffer cannot hold the entire packet, the overflow is discarded.

See `receive_async()` if you would like to receive a packet without halting your code until a packet is available.

#### Syntax

```C++
radio.receive_blocking(byte *buff, int buffMaxLen, uint32_t timeout)
```

#### Parameters

* _buff_: A user provided byte array for the packet payload to be copied into.  Contents of this array will be overwritten by this function.
* _buffMaxLen_: The maximum length of the buffer provided above.  This prevents a buffer overflow in the event that a received packet payload is larger than the buffer provided.
* _timeout_: (Optional) The maximum amount of time to wait for a packet in ms.  Set to `0` for no timeout (wait indefinitely).

#### Returns
* -1 When no packet is available, and we hit our timeout
* 0 When a packet with an empty payload has been received
* 1-255 when a non-empty payload has been received. This is the length of the packet payload.  This will always return the ACTUAL payload size, even if it is too large to fit in the user provided buffer.

#### Example

```C++
#include <LoraSx1262.h>

LoraSx1262 radio;
byte receiveBuff[255];

void setup() {
  Serial.begin(9600);

  if (!radio.begin()) { //Initialize the radio
    Serial.println("Failed to initialize radio");
  }
}

void loop() {
  //Wait up to 10 seconds for a packet to be received
  int bytesRead = radio.lora_receive_async(receiveBuff, sizeof(receiveBuff),10000);

  if (bytesRead < 0) {
    Serial.println("Timeout hit, no packet received);
  } else {
    //Print the payload out over serial
    Serial.print("Received: ");
    Serial.write(receiveBuff,bytesRead);
    Serial.println(); //Add a newline after printing
  }
}
```

#### See also

* [receive_async()](#receive_async)

### `configSetPreset()`

Change radio config using one of the pre-made configuration presets.  This is recommended for Beginner to Intermediate users.  Reminder: Both the transmitter and receiver must have identical configurations to be able to communicate with eachother.

Advanced users can change configurations individually instead of using presets.  See [configSetBandwidth()](#configSetBandwidth), [configSetCodingRate()](#configSetCodingRate), and [configSetSpreadingFactor()](#configSetSpreadingFactor) for details.

This does not change the radio frequency.  See [configSetFrequency()](#configSetFrequency).

Avalilable Presets:
* `PRESET_DEFAULT`: A simple multi-purpose preset for medium range and medium transmit speed.
  * Bandwidth: 250khz
  * Coding Rate: 4_5
  * Spreading Factor: 7
* `PRESET_LONGRANGE`: Longer range, slower speed.  Lower speeds allow long range communications to be more reliable
  * Bandwidth: 125khz
  * Coding Rate: 4_5
  * Spreading Factor: 12
* `PRESET_FAST`: High speed for short range. This works best when transmitter and receiver are closer together, allowing faster data speeds that would be unreliable at longer distances
  * Bandwidth: 500khz
  * Coding Rate: 4_5
  * Spreading Factor: 5

#### Example

```C++
#include <LoraSx1262.h>

LoraSx1262 radio;

void setup() {
  Serial.begin(9600);

  if (!radio.begin()) { //Initialize radio
    Serial.println("Failed to initialize radio.");
  }

  //Set radio to long-range preset
  radio.configSetPreset(PRESET_LONGRANGE);
}

void loop() {}
```

#### Syntax

```C++
radio.configSetPreset(int preset)
```

#### Parameters

* _preset_: Set to `PRESET_DEFAULT`, `PRESET_LONGRANGE`, or `PRESET_FAST`

#### Returns

`true` When a valid preset is specified
`false` When an invalid preset is specified

#### See also

* [configSetFrequency()](#configSetFrequency)
* [configSetBandwidth()](#configSetBandwidth)
* [configSetCodingRate()](#configSetCodingRate)
* [configSetSpreadingFactor()](#configSetSpreadingFactor)