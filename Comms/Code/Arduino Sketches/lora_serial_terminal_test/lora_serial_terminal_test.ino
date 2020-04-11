
// This sketch is a proof of concept to verify packet transmission of UART data
// from one LoRa radio to another. It pings out serial input to a radio to a pingback
// program and reports the packet that it receives in response.
// Currently under construction to add the ability to send command to change transmit
// settings for datarate

#include <SPI.h>
#include <RH_RF95.h>
#include <avr/dtostrf.h>
#include <RHReliableDatagram.h>


//------------------------DEBUG DEFINITIONS---------------------------------------------

#define DEBUG


// choose which hardware serial port by toggling debug mode
#ifdef DEBUG
auto hsp = Serial; // use USB serial in debug
#define debug_print(x) hsp.println(x); // provide method for tracing errors
#define debug_hold() while(!hsp){delay(1);};
#else
auto hsp = Serial1; // normal operation use UART
#endif

//-------------------------PIN DEFINITIONS------------------------------------------------

// For battery check, USB power check.
#define VBATPIN A7 // pin D9 on feather board
#define USB_PRESENT A2
#define USB_VOLTAGE 2.2   // half of maximum battery voltage. if USB pwr is gone
// the USB pin will read vBatt instead of 4.6-5.5.


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

//--------------------------Ping Time Variables--------------------------------------------


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

  pinMode(USB_PRESENT, INPUT);      // check to see if USB power is present
  pinMode(VBATPIN, INPUT);          // monitor battery level

  hsp.begin(9600);  // hsp = hardware serial port. usb in debug mode, uart in prod. mode.
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

  driver.setModemRegisters(&modem_config);      // give it the configuration we specified
  driver.setTxPower(23, false);                 // don't touch this. max output power, no PA_BOOST pin avail.
  rf95.setRetries(RETRIES);
  rf95.setTimeout(TIMEOUT);
}

//------------------Serial Input Methods---------------------------------------------

// set vars for serial input function
const byte numChars = 255;  // limits read to 255 bytes (full size of serial buffer
                            // and max length of a lora payload)
char msgBuffer[numChars];
boolean newData = false;
int packetSize;
int timeSent = 0;

void receiveLine() {
  static byte ndx = 0;
  char endMarker = '\n';
  char rc;
  while (hsp.available() > 0) {
    rc = hsp.read();

// -----------------------NEED ERROR CORRECTION IF INPUT > 253 CHARS-------------
    
    if ((rc != endMarker) && (ndx < (numChars - 2))) {
      msgBuffer[ndx] = rc;
      ndx++;
      if (ndx >= numChars) {
        ndx = numChars - 1;
      }
    }
    else {
      msgBuffer[ndx] = '\n';
      ndx++;
      msgBuffer[ndx] = '\0';
      ndx = 0;
      newData = true;
    }
  }
}

//----------------------LoRa methods---------------------------------------------

void transmit(char msg[], int msgLength){
  if (!rf95.sendtoWait((uint8_t *)msg, msgLength+1, 1)){
    hsp.println("Transmission failed. No ACK received.");
  }
  else{
    hsp.println("Transmission successful.");
  }
}

void transmitLine() {
  if (newData == true) {
    hsp.println(msgBuffer);
    newData = false;
    hsp.println("Transmitting...");             // Send a message to rf95_server
    transmit(msgBuffer, strlen(msgBuffer));  //send() needs uint8_t so we cast it
    timeSent = millis();
  }
}

void battLevel() {
  float vBatt = analogRead(VBATPIN);
  vBatt *= 2;           // undo voltage divider
  vBatt *= 3.3;         // full scale reference
  vBatt /= 1023;        // undo analogRead() 10bit integer scaling
  char cBuff[20];
  sprintf(cBuff, "%6.3f", vBatt); // cast voltage as c style string
  char voltagePacket[] = "";
  strcat(voltagePacket, "Current battery voltage: ");
  strcat(voltagePacket, cBuff);
  transmit(voltagePacket, strlen(voltagePacket));
}



void battCheck() {
  float cmpVoltage = analogRead(USB_PRESENT);
  cmpVoltage *= 3.3;
  cmpVoltage /= 1023;
  if (cmpVoltage < USB_VOLTAGE) {
    char helpMsg[numChars] = "";
    strcat(helpMsg, "The power system has failed. Please come find me. I'm scared.");
    transmit(helpMsg, strlen(helpMsg));
    battLevel();
    delay(30000);       // wait 30 seconds
  }
}

//--------------------------------Mainline-----------------------------------




void loop()
{
  if (hsp.available() == 0) {
    battCheck();            // check the power supply
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    uint8_t from;
    
    if (rf95.waitAvailableTimeout(1000)) {
      // Should be a reply message for us now
      if(rf95.recvfromAck(buf, &len, &from)){
        hsp.print("Message from client: "); hsp.println(from);
        hsp.println((char*)buf);
      }
  
      /*    
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
      */
      
    }
  }

  if (hsp.available() > 0) {
    receiveLine();
    transmitLine();
  }

}
