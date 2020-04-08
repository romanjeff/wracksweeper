// This sketch is a pingback program to respond to a ping from the LoRa radio
// It sends back the exact message that it receives.
 
#include <SPI.h>
#include <RH_RF95.h>
 

 
// for feather m0 RFM9x
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

 
/* Feather m0 w/wing 
#define RFM95_RST     11   // "A"
#define RFM95_CS      10   // "B"
#define RFM95_INT     6    // "D"
*/
 

 
 
// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0
 
// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);
 
// Blinky on receipt
#define LED 13



void setup()
{

  pinMode(LED, OUTPUT);
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
 
  Serial.begin(9600);


  while (!Serial) {
    delay(1);
  }


  delay(100);
  
  Serial.println("Feather LoRa Pinger-Backer!");

 
  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);
 
  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    Serial.println("Uncomment '#define SERIAL_DEBUG' in RH_RF95.cpp for detailed debug info");
    while (1);
  }
  Serial.println("LoRa radio init OK!");
 
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
 
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
 
  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
  // you can set transmitter powers from 5 to 23 dBm:



  RH_RF95::ModemConfig modem_config = {
  0x78, // Reg 0x1D: BW=125kHz, Coding=4/8, Header=explicit... bits 7-4 govern BW, 3-1 governs C.R.
        // LSB is header mode, leave as zero. 0x_2, 0x_4, 0x_6, 0x_8 represent C.R from 4/5->4/8. 
        // All other values of bits 3-0 reserved. Useable BW ranges from 0x5_ (31.25kHz) to 0x9_ (500kHz).
  0x94, // Reg 0x1E: Spread=512chips/symbol, CRC=enable...each bit increase in first nibble
        // of the byte corresponds to doubling of spreading factor (change 0x94 to 0xC4 for max)
        // Don't change bits 3-0. 
  0x0C  // Reg 0x26: LowDataRate=On, Agc=On... only bits 3 and 2 have meaning. bit 3 = mobile node?
        // bit 2 = automatic AGC?
  };

  
  //rf95.setModemConfig(RH_RF95::Bw125Cr48Sf4096);
  rf95.setModemRegisters(&modem_config);



  //set to maximum possible output power
  rf95.setTxPower(23, false);
}


 
void loop()
{
  
  if (rf95.available())
  {
    // Should be a message for us now
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
 
    if (rf95.recv(buf, &len))
    {

      digitalWrite(LED, HIGH);
      RH_RF95::printBuffer("Received: ", buf, len);
      Serial.print("Got: ");
      Serial.println((char*)buf);
       Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);

/*
      if(buf[0:6]=="setSlow"){
        rf95.setModemConfig(RH_RF95::Bw125Cr48Sf4096);
        Serial.println("Spreading factor 12, CRC 4/8");
        }
      }
*/
      // Send a reply
      uint8_t data[] = "You sent me ";
      rf95.send(data, sizeof(data));
      rf95.waitPacketSent();
      delay(10);
      rf95.send(buf, len);
      rf95.waitPacketSent();
      delay(10);
      Serial.println("Sent a reply");
      digitalWrite(LED, LOW);
    }
    else
    {

      Serial.println("Receive failed.");

    }
  }
}
