
// This sketch is a proof of concept to verify packet transmission of UART data
// from one LoRa radio to another. It pings out serial input to a radio to a pingback
// program and reports the packet that it receives in response.
// Currently under construction to add the ability to send command to change transmit
// settings for datarate

#include <SPI.h>
#include <RH_RF95.h>


#define DEBUG


// choose which hardware serial port by toggling debug mode
#ifdef DEBUG
auto hsp = Serial; // use USB serial in debug
#define debug_print(x) hsp.println(x); // provide method for tracing errors
#define debug_hold() while(!hsp){delay(1);};
#else
auto hsp = Serial1; // normal operation use UART
#endif



// for feather m0
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3




/* for shield
  #define RFM95_CS 10
  #define RFM95_RST 9
  #define RFM95_INT 7
*/


/* Feather m0 w/wing
  #define RFM95_RST     11   // "A"
  #define RFM95_CS      10   // "B"
  #define RFM95_INT     6    // "D"
*/


// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

unsigned long lastMsg;
unsigned long timeSince;
unsigned long longestBreak = 0;
int longestMin = 0;
int longestSec = 0;
int seconds;
int minutes;

void setup()
{
  lastMsg = millis();
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  hsp.begin(9600);  // Feather M0 hardware UART tx/rx are tied to "Serial1"
  // and not Serial, use as with Serial library.

  debug_hold();

  delay(100);


  hsp.println("Feather LoRa TX Test!");

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  //----------------------RFM95 Configuration Time-----------------------------------------

  while (!rf95.init()) {
    hsp.println("LoRa radio init failed");
    hsp.println("Uncomment '#define SERIAL_DEBUG' in RH_RF95.cpp for detailed debug info");
    while (1);
  }
  hsp.println("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    hsp.println("setFrequency failed");
    while (1);
  }
  hsp.print("Set Freq to: "); hsp.println(RF95_FREQ);

  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on



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

  rf95.setModemRegisters(&modem_config);



  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);
}

//------------------Serial Input Methods---------------------------------------------

// set vars for serial input function
const byte numChars = 255; //limits read to 255 bytes (full size of serial buffer
// and max length of a lora payload)
char payLoad[numChars];
boolean newData = false;
int packetSize;
int timeSent = 0;

void receiveLine() {

  static byte ndx = 0;
  char endMarker = '\n';
  char rc;


  while (hsp.available() > 0) {
    rc = hsp.read();
    if (rc != endMarker) {
      payLoad[ndx] = rc;
      ndx++;
      if (ndx >= numChars) {
        ndx = numChars - 1;
      }
    }
    else {
      payLoad[ndx] = '\0';
      packetSize = ndx + 1;
      ndx = 0;
      newData = true;
    }
  }
}


void sendLine() {
  if (newData == true) {
    hsp.println(payLoad);
    newData = false;
    hsp.println("Transmitting...");             // Send a message to rf95_server
    rf95.send((uint8_t *)payLoad,packetSize);   //send() needs uint8_t so we cast it
    rf95.waitPacketSent();
    timeSent = millis();
  }
}


//--------------------------------Mainline-----------------------------------




void loop()
{
  if (hsp.available() == 0) {
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.waitAvailableTimeout(1000)) {
      // Should be a reply message for us now
      if (rf95.recv(buf, &len)) {
        hsp.print((millis() - timeSent) / 1000.0);
        hsp.println(" seconds ping time.");
        hsp.println((char*)buf);
        hsp.print("RSSI: ");
        hsp.println(rf95.lastRssi(), DEC);
        hsp.print("SNR: ");
        hsp.println(rf95.lastSNR(), DEC);
      }

      else {
        hsp.println("Receive failed.");
        timeSent = millis();
      }

    }
  }

  if (hsp.available() > 0) {
    receiveLine();
    sendLine();
  }

}
