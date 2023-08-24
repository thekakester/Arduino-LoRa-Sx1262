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

#### Syntax

```C++
radio.configSetPreset(int preset)
```

#### Parameters

* _preset_: Set to `PRESET_DEFAULT`, `PRESET_LONGRANGE`, or `PRESET_FAST`

#### Returns

* `true` When a valid preset is specified
* `false` When an invalid preset is specified

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

#### See also

* [configSetFrequency()](#configSetFrequency)
* [configSetBandwidth()](#configSetBandwidth)
* [configSetCodingRate()](#configSetCodingRate)
* [configSetSpreadingFactor()](#configSetSpreadingFactor)

### `configSetFrequency()`

Change the operating frequency of the radio.  Transmitters and receivers must have matching frequencies (and radio config) to communicate with eachother.  Default frequency is 915mhz.

Changing the frequency allows radio communications between devices without interferfing with devices that are using a different frequency.  Reminder: the range of frequencies that are used by a radio is the base frequncy + the bandwidth. For example, 915mhz with 500khz (0.5mhz) bandwidth will use frequency ranges 914.75 - 915.25mhz.

*WARNING*: You must use frequencies that are legal in your country.  Search for "ISM Band Frequencies" to see available frequencies for your country.

This radio is compatible with frequencies from 150-960mhz.

Example:
* Region 2 (Americas): 902-928mhz

#### Syntax

```C++
radio.configSetFrequency(uint32_t frequency)
```

#### Parameters

* _frequency_: Frequency in hz. Eg use `915000000` for 915mhz.  This hardware supports 150-960mhz, but you must make sure that the frequency you choose is legal in your region.

#### Returns

* `true` When frequency set successfully
* `false` When an unsupported frequency is used (outside usable range)

#### Example

```C++
#include <LoraSx1262.h>

LoraSx1262 radio;

void setup() {
  Serial.begin(9600);

  if (!radio.begin()) { //Initialize radio
    Serial.println("Failed to initialize radio.");
  }

  //Set frequency to 913mhz
  radio.configSetFrequency(913000000);
}

void loop() {}
```

### `configSetBandwidth()`

Advanced configuration. This is recommended for users who are familiar with underlying radio concepts. Beginners are recommended to use presets with the [configSetPreset()](#configSetPreset) function.

#### Syntax

```C++
radio.configSetBandwidth(uint8_t bandwidthId)
```

#### Parameters

* _bandwidthId_: Bandwidth presets that are built-in to the radio.  See sx1262 datasheet section 13.4.5.2 for details.  Valid options:

| bandwidthId  | Bandwidth            |
| ------------ | -------------------- |
|    0x00      |    7.81khz           |
|    0x08      |   10.42khz           |
|    0x01      |   15.63khz           |
|    0x09      |   20.83khz           |
|    0x02      |   31.25khz           |
|    0x0A      |   41.67khz           |
|    0x03      |   62.50khz           |
|    0x04      |  125.00khz           |
|    0x05      |  250.00khz (default) |
|    0x06      |  500.00khz           |

#### Returns

* `true` When bandwidth is set successfully
* `false` When an invalid bandwitdh identifier is used

#### Example

```C++
#include <LoraSx1262.h>

LoraSx1262 radio;

void setup() {
  Serial.begin(9600);

  if (!radio.begin()) { //Initialize radio
    Serial.println("Failed to initialize radio.");
  }

  //Set Bandwidth to 500khz
  radio.configSetBandwidth(0x06);
}

void loop() {}
```

#### See also

* [configSetPreset()](#configSetFrequency)
* [configSetBandwidth()](#configSetBandwidth)
* [configSetCodingRate()](#configSetCodingRate)
* [configSetSpreadingFactor()](#configSetSpreadingFactor)

### `configSetCodingRate()`

Advanced configuration. This is recommended for users who are familiar with underlying radio concepts. Beginners are recommended to use presets with the [configSetPreset()](#configSetPreset) function.

Coding rate increases the packet size to improve the reception of messages. [Learn More](https://www.thethingsnetwork.org/docs/lorawan/fec-and-code-rate/)

#### Syntax

```C++
radio.configSetCodingRate(uint8_t codingRateId)
```

#### Parameters

* _codingRateId_: Coding Rate presets that are built-in to the radio.  See sx1262 datasheet section 13.4.5.2 for details.  Valid options:

| codingRateId | Coding Rate        |
| ----------   | ----------------   |
|    0x01      |   CR_4_5 (default) |
|    0x02      |   CR_4_6           |
|    0x03      |   CR_4_7           |
|    0x04      |   CR_4_8           |

#### Returns

* `true` When coding rate is set successfully
* `false` When an invalid coding rate identifier is used

#### Example

```C++
#include <LoraSx1262.h>

LoraSx1262 radio;

void setup() {
  Serial.begin(9600);

  if (!radio.begin()) { //Initialize radio
    Serial.println("Failed to initialize radio.");
  }

  //Set Coding Rate to CR_4_6
  radio.configSetCodingRate(0x02);
}

void loop() {}
```

#### See also
* [configSetPreset()](#configSetFrequency)
* [configSetBandwidth()](#configSetBandwidth)
* [configSetCodingRate()](#configSetCodingRate)
* [configSetSpreadingFactor()](#configSetSpreadingFactor)

### `configSetSpreadingFactor()`

Advanced configuration. This is recommended for users who are familiar with underlying radio concepts. Beginners are recommended to use presets with the [configSetPreset()](#configSetPreset) function.

Spreading factor changes the chirp rate.
Generally Speaking:
* High SpreadingFactor: Faster data rate, less reliable at long ranges
* Low SpreadingFactor: Lower data rate, more reliable at long ranges

[Learn More](https://lora-developers.semtech.com/documentation/tech-papers-and-guides/lora-and-lorawan/)

#### Syntax

```C++
radio.configSetSpreadingFactor(uint8_t spreadingFactorId)
```

#### Parameters

* _spreadingFactorId_: Spreading Factor presets that are built-in to the radio.  See sx1262 datasheet section 13.4.5.2 for details.  Valid options:

| spreadingFactorId | Spreading Factor                          |
| ------------------| ----------------------------------------- |
|         5         | SF5 (fastest, short range)                |
|         6         | SF6                                       |
|         7         | SF7 (default)                             |
|         8         | SF8                                       |
|         9         | SF9                                       |
|        10         | SF10                                      |
|        11         | SF11                                      |
|        12         | SF12 (Slowest, long range, most reliable) |

#### Returns

* `true` When spreading factor is set successfully
* `false` When an invalid spreading factor identifier is used

#### Example

```C++
#include <LoraSx1262.h>

LoraSx1262 radio;

void setup() {
  Serial.begin(9600);

  if (!radio.begin()) { //Initialize radio
    Serial.println("Failed to initialize radio.");
  }

  //Set Spreading Factor to SF12 (most reliable, but slowest)
  radio.configSetSpreadingFactor(12);
}

void loop() {}
```

#### See also

* [configSetPreset()](#configSetFrequency)
* [configSetBandwidth()](#configSetBandwidth)
* [configSetCodingRate()](#configSetCodingRate)
* [configSetSpreadingFactor()](#configSetSpreadingFactor)