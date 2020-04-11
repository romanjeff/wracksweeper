// This sketch is a pingback program to respond to a ping from the LoRa radio
// It sends back the exact message that it receives.
 
#include <SPI.h>
#include <RH_RF95.h>
#include <RHReliableDatagram.h>
#include <stdlib.h>  // for strtok() function to split strings
#include <string.h>  // for strstr() stringsearch
 
#define DEBUG


// choose which hardware serial port by toggling debug mode
#ifdef DEBUG
auto hsp = Serial; // use USB serial in debug
#define debug_print(x) hsp.println(x); // provide method for tracing errors
#define debug_hold() while(!hsp){delay(1);};
#else
auto hsp = Serial1; // normal operation use UART
#endif

 
// for feather m0 RFM9x
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

 
/* Feather m0 w/wing 
#define RFM95_RST     11   // "A"
#define RFM95_CS      10   // "B"
#define RFM95_INT     6    // "D"
*/
 

 
//--------------------------LORA SETTINGS DEFS---------------------------------------------

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0
#define TIMEOUT 4000
#define RETRIES 5
#define SERVER_ADDRESS 1            // AUV radio is the server
#define CLIENT_ADDRESS 2            // shore pc is the client

// Singleton instance of the radio driver
RH_RF95 driver(RFM95_CS, RFM95_INT);                // driver is radio hardware driver
RHReliableDatagram rf95(driver,SERVER_ADDRESS);     // rf95 is the msg manager

 
// Blinky on receipt
#define LED 13



void setup()
{

  pinMode(LED, OUTPUT);
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
 
  hsp.begin(9600);
  debug_hold();
  delay(100);
  
  hsp.println("Feather LoRa Pinger-Backer!");

 
  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);
 
  while (!rf95.init()) {
    hsp.println("LoRa radio init failed");
    hsp.println("Uncomment '#define SERIAL_DEBUG' in RH_RF95.cpp for detailed debug info");
    while (1);
  }
  hsp.println("LoRa radio init OK!");
 
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!driver.setFrequency(RF95_FREQ)) {
    hsp.println("setFrequency failed");
    while (1);
  }
  hsp.print("Set Freq to: "); hsp.println(RF95_FREQ);
 
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
  // Change this to taste by editing modem_config below:



  RH_RF95::ModemConfig modem_config = {
  0x72, // Reg 0x1D: BW=125kHz, Coding=4/8, Header=explicit... bits 7-4 govern BW, 3-1 governs C.R.
        // LSB is header mode, leave as zero. 0x_2, 0x_4, 0x_6, 0x_8 represent C.R from 4/5->4/8. 
        // All other values of bits 3-0 reserved. Useable BW ranges from 0x5_ (31.25kHz) to 0x9_ (500kHz).
  0x84, // Reg 0x1E: Spread=512chips/symbol, CRC=enable...each bit increase in first nibble
        // of the byte corresponds to doubling of spreading factor (change 0x94 to 0xC4 for max)
        // Don't change bits 3-0. 
  0x0C  // Reg 0x26: LowDataRate=On, Agc=On... only bits 3 and 2 have meaning. bit 3 = mobile node?
        // bit 2 = automatic AGC?
  };

  
  //rf95.setModemConfig(RH_RF95::Bw125Cr48Sf4096);
  driver.setModemRegisters(&modem_config);
  
  //set to maximum possible output power
  driver.setTxPower(23, false);

  rf95.setRetries(RETRIES);
  rf95.setTimeout(TIMEOUT);
}

void transmit(char msg[], int msgLength){
  if (!rf95.sendtoWait((uint8_t *)msg, msgLength + 1, 1)){
    hsp.println("Transmission failed. No ACK received.");
  }
  else{
    hsp.println("Transmission successful.");
  }
}

 
void loop()
{
  
  if (rf95.available())
  {
    // Should be a message for us now
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    uint8_t from;
    if(rf95.recvfromAck(buf, &len, &from)){
        hsp.print("Message from client: "); hsp.println(from);
        hsp.println((char*)buf);
        
    }
    else{
      hsp.println("Receive failed.");
    }
      
    /*
    if (rf95.recv(buf, &len))
    {
      char *s; // pointer for string search


      digitalWrite(LED, HIGH);
      //RH_RF95::printBuffer("Received: ", buf, len);
      //hsp.print("Got: ");
      hsp.println((char*)buf);
      hsp.print("RSSI: ");
      hsp.println(rf95.lastRssi(), DEC);
      hsp.print("SNR: ");
      hsp.println(rf95.lastSNR(), DEC);


      // Send a reply
      uint8_t data[] = "You sent me ";
      rf95.send(data, sizeof(data));
      rf95.waitPacketSent();
      delay(10);
      rf95.send(buf, len);
      rf95.waitPacketSent();
      delay(10);
      hsp.println("Sent a reply");
      digitalWrite(LED, LOW);
    }
    else
    {
      hsp.println("Receive failed.");
    }
    */
  }
}
