/*
   RadioLib CC1101 Receive with Interrupts Example

   This example listens for FSK transmissions and tries to
   receive them. Once a packet is received, an interrupt is
   triggered.

   To successfully receive data, the following settings have to be the same
   on both transmitter and receiver:
    - carrier frequency
    - bit rate
    - frequency deviation
    - sync word

   For default module settings, see the wiki page
   https://github.com/jgromes/RadioLib/wiki/Default-configuration#cc1101

   For full API reference, see the GitHub Pages
   https://jgromes.github.io/RadioLib/
*/

// include the library
#include <RadioLib.h>

// CC1101 has the following connections:
// CS pin:    15/D8(ESP8266)
// GDO0 pin:  5/D1(ESP8266)
// RST pin:   unused
// GDO2 pin:  3 (optional)
CC1101 radio = new Module(15, 5, RADIOLIB_NC, 3);  // cs, gdo0, lib, gdo2

// enabled godmode in build.h

void setup() {
  Serial.begin(9600);
  delay(500);
  Serial.println(); Serial.println();
  delay(500);

  // initialize CC1101 with default settings
  Serial.print(F("[CC1101] Initializing ... "));
  int state = radio.begin();
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true);
  }
    
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
    Serial.print(F("[CC1101] Partnumber ")); Serial.println(radio.SPIgetRegValue(0x30), HEX);
    Serial.print(F("[CC1101] Version ")); Serial.println(radio.getChipVersion(), HEX);
    Serial.println();

    float frequency = 433.92;
    if (radio.setFrequency(frequency) == RADIOLIB_ERR_INVALID_FREQUENCY) {
      Serial.println(F("[CC1101] Selected frequency is invalid for this module!"));
      while (true);
    } else {
      Serial.print(F("[CC1101] setFrequency [MHz] ")); Serial.println(frequency);
    }
    float bitrate = 19.2;  // 19200 baud
    state = radio.setBitRate(bitrate);
    if (state == RADIOLIB_ERR_INVALID_BIT_RATE) {
      Serial.println(F("[CC1101] Selected bit rate is invalid for this module!"));
      while (true);
    } else if (state == RADIOLIB_ERR_INVALID_BIT_RATE_BW_RATIO) {
      Serial.println(F("[CC1101] Selected bit rate to bandwidth ratio is invalid!"));
      Serial.println(F("[CC1101] Increase receiver bandwidth to set this bit rate."));
      while (true);
    } else {
      Serial.print(F("[CC1101] setBitRate [kbps] ")); Serial.println(bitrate);
    }
    float bandwidth = 135.0;
    if (radio.setRxBandwidth(bandwidth) == RADIOLIB_ERR_INVALID_RX_BANDWIDTH) {
      Serial.println(F("[CC1101] Selected receiver bandwidth is invalid for this module!"));
      while (true);
    } else {
      Serial.print(F("[CC1101] setBandwidth [kHz] ")); Serial.println(bandwidth)
    }
    radio.fixedPacketLengthMode(9);
  
    byte syncHigh = 0x00; byte syncLow = 0x1a;
    radio.SPIwriteRegister(0x04, syncHigh); Serial.print(F("[CC1101] Reg0x04 SetSyncHigh ")); Serial.println(syncHigh);
    radio.SPIwriteRegister(0x05, syncLow); Serial.println(F("[CC1101] Reg0x05 SetSyncLow ")); Serial.println(syncLow);

    radio.setEncoding(RADIOLIB_ENCODING_MANCHESTER);

  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true);
  }

  // set the function that will be called
  // when new packet is received
  radio.setGdo0Action(setFlag);

  // start listening for packets
  Serial.print(F("[CC1101] Starting to listen ... "));
  state = radio.startReceive();
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true);
  }
  Serial.println();
}

// flag to indicate that a packet was received
volatile bool receivedFlag = false;

// disable interrupt when it's not needed
volatile bool enableInterrupt = true;

// this function is called when a complete packet
// is received by the module
// IMPORTANT: this function MUST be 'void' type
//            and MUST NOT have any arguments!
#if defined(ESP8266) || defined(ESP32)
  ICACHE_RAM_ATTR
#endif
void setFlag(void) {
  // check if the interrupt is enabled
  if(!enableInterrupt) {
    return;
  }

  // we got a packet, set the flag
  receivedFlag = true;
}

void printhex(int h) {
  if (h<16) {
    Serial.print("0");
  }
  Serial.print(h,HEX);
}

void loop() {
  // check if the flag is set
  if(receivedFlag) {
    // disable the interrupt service routine while
    // processing the data
    enableInterrupt = false;

    // reset flag
    receivedFlag = false;

    // you can read received data as an Arduino String

    // you can also read received data as byte array
    int len = radio.getPacketLength();
    byte byteArr[len+1];
    int state = radio.readData(byteArr, len);

    Serial.print(len); Serial.print("; ");
    Serial.print(radio.getRSSI());
    Serial.print("; ");
    Serial.print(radio.getLQI());
    Serial.print("; ");

    // print data of the packet
    for (int i=0; i<len; i++) {
      printhex(byteArr[i]);
    }

    int chksum = (byteArr[0]^byteArr[1]^byteArr[2]^byteArr[3]^byteArr[4]^byteArr[5]^byteArr[6]^byteArr[7]);
    if (chksum == byteArr[8]) {
      Serial.print("  wheel: "); Serial.print(byteArr[4],DEC);
      Serial.print("  press: "); Serial.print((byteArr[5]&0xf)*256+byteArr[6]);
      Serial.print("  temp: "); Serial.print(byteArr[7]);
    } else { Serial.print("  wrong checksum");
    }
    
    Serial.println();

    // put module back to listen mode
    radio.startReceive();

    // we're ready to receive more packets,
    // enable interrupt service routine
    enableInterrupt = true;
  }
}
