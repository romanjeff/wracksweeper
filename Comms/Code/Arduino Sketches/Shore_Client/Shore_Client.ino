// AUV LoRa Link Arduino Sketch
// This revision: v.1.0;
// Original authorship: Jeff Roman, 04/2020
// PSU Capstone Team: Wracksweeper/WAVES

// Designed for use with Adafruit Feather M0 packaged with onboard HopeRF RFM95 LoRa module.
// Battery monitoring provided by setting a voltage divider of 2 2.2k resistors from the
// "USB" pin to analog pin A2 to ground.

// CURRENT FUNCTIONALITY:
// This is the Shore Client-enabled sketch of the terminal-over-LoRa software which enables
// a team to remotely query and direct the MOOS community running in a surfaced AUV within
// range of the RF link. It transmits any serial input up to 100 bytes (more possible based
// on specific link settings) to a shore client, and conveys any received data as UART
// output when set as an AUV/server. When set as shore client, it provides wireless data
// transmission of all serial input up to 100 bytes and returns any response. In shore client
// mode, there is a feature LINKMON which also includes data on RF link quality (RSSI, SNR).

// There is a method -changemode which can be invoked from either side that can allow
// a switch to different RF link settings for faster datarate in better link environments or
// slower transfer in areas with poor link quality. All messaging is ACK-based and CRC'ed,
// with CRC coding settable with -changemode.

// The AUV server mode is intended to interface with a custom UART daemon which properly
// deals with any of the valid inputs.

// The shore client mode is intended to interface with a CLI-based python script which will
// display received messages and allow the user to choose to send any output along to a local
// MOOS community.

// DESIRED FUTURE FUNCTIONALITY:
// 1) A getGPS procedure for inclusion and regular transmission from AUV while it's floating.
//    This will require a query to BBB, with the last known GPS sentence stored in case BBB
//    dies for inclusion in help message transmissions.
// 2) A "help" dialogue which can be entered from the shore client command line to access
//    available calls and get guidance.
// 3) A more extensible flag system for non-terminal methods like -changemode that only affect
//    the arduinos.
// 4) A buffer handler to enable data of arbitrary size to be transmitted with full reliable
//    datagram accuracy, enabling "upload" and "download" capabilities.



//---------WARNING-----------WARNING-------------WARNING-------------WARNING--------------
//------------------------THIS SKETCH IS THE CLIENT(ADDRESS 2)----------------------------
//-------------IT IS IMPORTANT NOT TO USE THE SAME SKETCH FOR BOTH RADIOS-----------------



#include <SPI.h>
#include <RH_RF95.h> // this and RHReliableDatagram from https://github.com/adafruit/RadioHead
#include <avr/dtostrf.h>
#include <RHReliableDatagram.h>
#include <string.h>
#include <MemoryFree.h>  // get from https://github.com/mpflaga/Arduino-MemoryFree 

//------------------------DEBUG DEFINITIONS---------------------------------------------



//============WARNING====================WARNING===============WARNING==================
// COMMENT OUT THE DEFINITION OF DEBUG FOR USE INSIDE AUV. DEBUG MODE DIRECTS ALL SERIAL
// DATA TO USB INSTEAD OF UART! THIS IS IMPORTANT!


#define DEBUG

// choose which hardware serial port by toggling debug mode
#ifdef DEBUG
bool db = true;
auto hsp = Serial; // use USB serial in debug
#define debug_print(x) hsp.println(x); // provide method for tracing errors
#define debug_hold() while(!hsp){delay(1);};
#else
bool db = false;
auto hsp = Serial1; // normal operation use UART
#define debug_print(x)
#define debug_hold()
#endif

//-------------------------PIN DEFINITIONS------------------------------------------------

// For battery check, USB power check.
#define VBATPIN A7 // pin D9 on feather board
#define USB_PRESENT A2
#define USB_VOLTAGE 2.2   // half of maximum battery voltage. if USB pwr is gone
// the USB pin will read vBatt instead of 4.6-5.5.


#define LED 13


// for feather m0
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3


//--------------------------LORA SETTINGS DEFS---------------------------------------------

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0
#define TIMEOUT 8000
#define RETRIES 5


#define SHORE_CLIENT            /* UNCOMMENT FOR SHORE CLIENT */

#ifdef SHORE_CLIENT
const bool SHORE = true;
#define MY_ADDRESS 2                // shore pc is the client
#define REMOTE_ADDRESS 1
#else
const bool SHORE = false;
#define MY_ADDRESS 1
#define REMOTE_ADDRESS 2
#endif

// EFFECTS OF CHANGING BW: -3dB link budget for each doubling of BW
// but datarate can effectively double.
// EFFECTS of CHANGING SF: +2.5dB link budget for each increase of SF
// but datarate cuts in half.
// EFFECTS OF CHANGING CR: Datarate penalty relative to how many chars
// are used just for CRC vs. how many used for payload.

// Fastest Packet time - bw500cr45 sf128, ~0.1s
// Slowest Packet time - bw125cr48 sf4096, ~7s

// Pending distance testing, recommend default bw125cr48 sf512 for best
// balance of link budget, reliability, and datarate. ~1-2s packet time.

// SETTING OPTIONS FOR REGISTER 1D

const byte bw125cr45 = 0x72;
const byte bw125cr46 = 0x74;
const byte bw125cr47 = 0x76;
const byte bw125cr48 = 0x78;
const byte bw250cr45 = 0x82;
const byte bw250cr46 = 0x84;
const byte bw250cr47 = 0x86;
const byte bw250cr48 = 0x88;
const byte bw500cr45 = 0x92;
const byte bw500cr46 = 0x94;
const byte bw500cr47 = 0x96;
const byte bw500cr48 = 0x98;


// SETTING OPTIONS FOR REGISTER 1E

const byte sf128 = 0x74;
const byte sf256 = 0x84;
const byte sf512 = 0x94;
const byte sf1024 = 0xA4;
const byte sf2048 = 0xB4;
const byte sf4096 = 0xC4;

typedef struct {
  char *key;
  byte val;
} t_symstruct;   // create struct for setting LUTs

static t_symstruct bwLUT[] = {
  {"bw125cr45\0", bw125cr45}, {"bw250cr45\0", bw250cr45}, {"bw500cr45\0", bw500cr45},
  {"bw125cr46\0", bw125cr46}, {"bw250cr46\0", bw250cr46}, {"bw500cr46\0", bw500cr46},
  {"bw125cr47\0", bw125cr47}, {"bw250cr47\0", bw250cr47}, {"bw500cr47\0", bw500cr47},
  {"bw125cr48\0", bw125cr48}, {"bw250cr48\0", bw250cr48}, {"bw500cr48\0", bw500cr48}
};

static t_symstruct sfLUT[] = {
  {"sf128\0", sf128}, {"sf256\0", sf256}, {"sf512\0", sf512},
  {"sf1024\0", sf1024}, {"sf2048\0", sf2048}, {"sf4096\0", sf4096}
};

#define N_bwKeys 12
#define N_sfKeys 6


// Singleton instance of the radio driver
RH_RF95 driver(RFM95_CS, RFM95_INT);                // driver is radio hardware driver
RHReliableDatagram rf95(driver, MY_ADDRESS);    // rf95 is the msg manager



//----WARNING-----------WARNING-------------WARNING-------------WARNING-------------------
//------------------------THIS SKETCH IS THE CLIENT(ADDRESS 2)----------------------------
//-------------IT IS IMPORTANT NOT TO USE THE SAME SKETCH FOR BOTH RADIOS-----------------


void ledFlash(int n) {
  int i = 0;
  while (i < n) {
    digitalWrite(LED, HIGH);
    delay(100);
    digitalWrite(LED, LOW);
    delay(100);
    i++;
  }
}

//---------------------------------SETUP-----------------------------------------------------



void setup()
{
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);


  ledFlash(2);
  hsp.begin(9600);

  debug_hold();
  delay(100);
  if (SHORE) {
    hsp.println("Welcome to the AUV Remote Command Center.");
    hsp.println("Please limit packets to 100 characters. Type and press enter to send.");
  }
  else {
    debug_print("I am an AUV. This is how I talk!");
  }

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  //-------------------------RFM95 Configuration Time------------------

  while (!rf95.init()) {
    debug_print("LoRa radio init failed");
    debug_print("Uncomment '#define SERIAL_DEBUG' in RH_RF95.cpp for detailed debug info");
    while (1) {
      ledFlash(1);
    }
  }
  debug_print("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!driver.setFrequency(RF95_FREQ)) {
    debug_print("setFrequency failed");
    while (1) {
      ledFlash(1);
    }
  }
  debug_print("Set Freq to: "); debug_print(RF95_FREQ);

  // Initial transmission settings
  // DEFAULT: FASTEST DATARATE AVAILABLE

  RH_RF95::ModemConfig modem_config = {
    bw500cr45,
    sf128,
    0x0C  // LEAVE THIS ALONE
  };

  driver.setModemRegisters(&modem_config);

  //set to maximum possible output power
  driver.setTxPower(23, false);

  rf95.setRetries(RETRIES);
  rf95.setTimeout(TIMEOUT);
}

//------------------------------------------------------------------------------------------

//----------------------------Global Variables----------------------------------------------

// set vars for serial input function
const byte numChars = 255;  // limits read to 255 bytes (full size of serial buffer
// and max length of a lora payload)
const int totalMem = 262144;
byte msgBuffer[numChars];   // Serial input buffer
boolean newData = false;
byte set[3];      // global array to hold the settings
byte buf[numChars];      // receive buffer
byte len = sizeof(buf);
byte from;
// command keywords
const char MODE[] = "-changemode\0";
const char RCVD[] = "-received-\0";
int memDiff;
int lastMem = freeMemory();
float memPct;
int timer = millis();

//------------------Serial Input Methods----------------------------------------------------

void receiveLine() {
  static byte ndx = 0;
  byte endMarker = '\n';
  byte rc;
  while (hsp.available() > 0) {
    rc = hsp.read();

    // -----------------------STILL NEED ERROR CORRECTION IF INPUT > 253 CHARS-------------

    if ((rc != endMarker) && (ndx < (numChars - 2))) {
      msgBuffer[ndx] = rc;
      ndx++;
      if (ndx >= numChars) {
        ndx = numChars - 1;
      }
    }
    else {
      msgBuffer[ndx] = endMarker;
      ndx++;
      msgBuffer[ndx] = '\0';
      ndx = 0;
      newData = true;
    }
  }
}

//------------------Look Up Table Access for Settings ------------------------------------

int keyfromstring(char *key, int param)  // param = 0 for bw/cr, 1 for sf
{
  int NKEYS;
  if (param == 2) {
    NKEYS = N_sfKeys;
    for (int i = 0; i < NKEYS; i++) {
      if (strcmp(sfLUT[i].key, key) == 0) {
        debug_print("matched.")
        debug_print(key);
        return sfLUT[i].val;
      }
    }
  }
  else {
    NKEYS = N_bwKeys;
    for (int i = 0; i < NKEYS; i++) {
      if (strcmp(bwLUT[i].key, key) == 0) {
        debug_print("matched.")
        debug_print(key);
        return bwLUT[i].val;
      }
    }
  }
  return -1;
}

//------------------------------LoRa methods---------------------------------------------

void transmit(byte msg[], int msgLength) {
  int packetTime = millis();
  if (!rf95.sendtoWait((uint8_t *)msg, msgLength + 1, REMOTE_ADDRESS)) {
    debug_print("Transmission failed. No ACK received.");
  }

  else {
    if (SHORE) {
      packetTime = millis() - packetTime;
      hsp.println("Transmission successful. Bytes sent: ");
      for (int i = 0; i <= msgLength; i++) {
        hsp.print(msg[i], HEX);
      }
      hsp.print("\n");
      hsp.print(packetTime / 1000.0); hsp.println(" seconds packet time.");
    }
  }
}

//----------------------------Initiate Settings Change Handshake-------------------

int modeChangeLocal(byte msg[]) {
  set[2] = 0;
  int c = 0;
  char msgCopy[numChars];
  strcpy(msgCopy, (char*)msg);
  char* rest = msgCopy;
  char* token = strtok(rest, "\n");       // clean off newline
  token = strtok(rest, "\r");       // clean off carriage return

  while (token = strtok_r(rest, " ", &rest)) {
    if (c == 1) {
      switch (keyfromstring(token, c)) {  // param 0 corresponds to bw,cr parameter
        case -1:
          hsp.println("not a valid setting.");
          hsp.println("Try again with the settings format -changemode bw___cr__ sf___");
          set[2] = 1;    // must check set[2] to ensure valid overall settings
          break;
        case bw125cr45: set[0] = bw125cr45; break;
        case bw125cr46: set[0] = bw125cr46; break;
        case bw125cr47: set[0] = bw125cr47; break;
        case bw125cr48: set[0] = bw125cr48; break;
        case bw250cr45: set[0] = bw250cr45; break;
        case bw250cr46: set[0] = bw250cr46; break;
        case bw250cr47: set[0] = bw250cr47; break;
        case bw250cr48: set[0] = bw250cr48; break;
        case bw500cr45: set[0] = bw500cr45; break;
        case bw500cr46: set[0] = bw500cr46; break;
        case bw500cr47: set[0] = bw500cr47; break;
        case bw500cr48: set[0] = bw500cr48; break;
      }
    }

    if (c == 2) {
      switch (keyfromstring(token, c)) {
        case -1:
          hsp.println("not a valid setting.");
          hsp.println("Try again with the settings format -changemode bw___cr__ sf___");
          set[2] = 1;    // must check set[2] to ensure valid overall settings
          break;
        case sf128: set[1] = sf128; break;
        case sf256: set[1] = sf256; break;
        case sf512: set[1] = sf512; break;
        case sf1024: set[1] = sf1024; break;
        case sf2048: set[1] = sf2048; break;
        case sf4096: set[1] = sf4096; break;
      }
    }
    c++;
  }

  if (set[2] == 1) {
    return -1;
  }

  if (!rf95.sendtoWait(msg, strlen((char*)msg) + 1, REMOTE_ADDRESS)) {
    hsp.println("No Ack received, cannot change mode.");
    return -1;
  }

  else {
    return 0;
  }
}

//----------------Handshake to settings change from other side of RF Link-------------------

int modeChangeRemote(byte msg[]) {

  set[2] = 0;
  int sC = 0;
  debug_print("The char array looks like: ");
  debug_print((char*)msg);
  char msgCopy[numChars];
  char newBWCR[] = "";
  char newSF[] = "";
  strcpy(msgCopy, (char*)msg);
  char* rest = msgCopy;         // pointer for tokenizing
  char* token = strtok(rest, " ");
  while (token != NULL) {
    switch (keyfromstring(token, 1)) {  // param 0 corresponds to bw,cr parameter
      case -1: break;
      case bw125cr45: set[0] = bw125cr45; strcat(newBWCR, "125kHz, 4_5, "); sC++; break;
      case bw125cr46: set[0] = bw125cr46; strcat(newBWCR, "125kHz, 4_6, "); sC++; break;
      case bw125cr47: set[0] = bw125cr47; strcat(newBWCR, "125kHz, 4_7, "); sC++; break;
      case bw125cr48: set[0] = bw125cr48; strcat(newBWCR, "125kHz, 4_8, "); sC++; break;
      case bw250cr45: set[0] = bw250cr45; strcat(newBWCR, "250kHz, 4_5, "); sC++; break;
      case bw250cr46: set[0] = bw250cr46; strcat(newBWCR, "250kHz, 4_6, "); sC++; break;
      case bw250cr47: set[0] = bw250cr47; strcat(newBWCR, "250kHz, 4_7, "); sC++; break;
      case bw250cr48: set[0] = bw250cr48; strcat(newBWCR, "250kHz, 4_8, "); sC++; break;
      case bw500cr45: set[0] = bw500cr45; strcat(newBWCR, "500kHz, 4_5, "); sC++; break;
      case bw500cr46: set[0] = bw500cr46; strcat(newBWCR, "500kHz, 4_6, "); sC++; break;
      case bw500cr47: set[0] = bw500cr47; strcat(newBWCR, "500kHz, 4_7, "); sC++; break;
      case bw500cr48: set[0] = bw500cr48; strcat(newBWCR, "500kHz, 4_8, "); sC++; break;
    }
    switch (keyfromstring(token, 2)) {
      case -1: break;
      case sf128: set[1] = sf128; strcat(newSF, "128\0"); sC++; break;
      case sf256: set[1] = sf256; strcat(newSF, "256\0"); sC++; break;
      case sf512: set[1] = sf512; strcat(newSF, "512\0"); sC++; break;
      case sf1024: set[1] = sf1024; strcat(newSF, "1024\0"); sC++; break;
      case sf2048: set[1] = sf2048; strcat(newSF, "2048\0"); sC++; break;
      case sf4096: set[1] = sf4096; strcat(newSF, "4096\0"); sC++; break;
    }
    token = strtok(NULL, " ");
  }

  int c = 0;
  debug_print("Set[0-2] =");
  while (c < 3) {
    debug_print(set[c]);
    c++;
  }
  if (sC < 2) {
    debug_print("That didn't work.");
    debug_print("Try again with the settings format -changemode bw___cr__ sf___");
    return -1;
  }
  char confirmation[numChars] = "";
  strcat(confirmation, "-received- New settings: ");
  strcat(confirmation, newBWCR);
  strcat(confirmation, newSF);
  debug_print(newBWCR);
  debug_print(newSF);
  debug_print(confirmation);
  delay(100);
  if (!rf95.sendtoWait((uint8_t *)confirmation, strlen(confirmation) + 1, REMOTE_ADDRESS)) {
    debug_print("No Ack received, cannot change mode.");
    return -1;
  }
  else {
    debug_print("Acknowledge received. Changing mode now.");

    RH_RF95::ModemConfig new_modem_config = {
      set[0],
      set[1],
      0x0C  // LEAVE THIS ALONE
    };
    delay(10);
    driver.setModemRegisters(&new_modem_config);      // give it the configuration we specified
    delay(10);
    if (db) {
      driver.printRegisters();
    }

  }
  return 0;
}

//----------------Transmit received Serial data or initiate settings change locally-----------

void transmitLine() {
  if (newData == true) {
    debug_print((char*)msgBuffer);
    newData = false;
    char tmp[numChars];
    strcpy(tmp, (char*)msgBuffer);
    char *token = strtok(tmp, " ");
    if (strcmp(token, MODE) == 0) {
      if (modeChangeLocal(msgBuffer) == 0) {
        hsp.println("Acknowledge received. Will change settings once handshake is complete.");
      }
      else {
        hsp.println("Attempt to change transmission settings failed.");
      }
    }
    else {
      debug_print("Transmitting...");             // Send a message to rf95_server
      transmit(msgBuffer, strlen((char*)msgBuffer));  //send() needs uint8_t so we cast it
    }
  }
}

//--------------------Battery Monitoring Functions-----------------------------



void battCheck() {
  float cmpVoltage = analogRead(USB_PRESENT);
  cmpVoltage *= 3.3;
  cmpVoltage /= 1023;
  if (cmpVoltage < USB_VOLTAGE) {
    char helpMsg[numChars] = "";
    strcat(helpMsg, "The power system has failed. Please come find me. I'm scared. ");
    float vBatt = analogRead(VBATPIN);
    vBatt *= 2;           // undo voltage divider
    vBatt *= 3.3;         // full scale reference
    vBatt /= 1023;        // undo analogRead() 10bit integer scaling
    char cBuff[12];
    sprintf(cBuff, "%6.3f", vBatt); // cast voltage as c style string
    strcat(helpMsg, "Current battery voltage: ");
    strcat(helpMsg, cBuff);
    transmit((uint8_t*)helpMsg, strlen(helpMsg));
    timer = millis();       // wait 30 seconds
  }
}

//--------------------------------Mainline-------------------------------------------

void loop()
{

  if (hsp.available() > 0) {
    digitalWrite(LED, HIGH);
    receiveLine();
    transmitLine();
    digitalWrite(LED, LOW);
  }


  if (hsp.available() == 0)
  {
    if (!SHORE) {
      if (millis() - timer > 30000) {
        battCheck();            // check the power supply
      }
    }
    // Should be a message for us now
    uint8_t buf[numChars];
    uint8_t len = sizeof(buf);
    uint8_t from;

    if (rf95.available()) {
      if (rf95.recvfromAck(buf, &len, &from)) {
        if (SHORE) {          // if shore client, give rf link info and packet time as well as message
          ledFlash(2);
          hsp.println((char*)buf);

          hsp.print("RSSI: ");
          hsp.println(driver.lastRssi(), DEC);
          hsp.print("SNR: ");
          hsp.println(driver.lastSNR(), DEC);
          int freeMem = freeMemory();
          memDiff = lastMem - freeMem;
          lastMem = freeMem;
          hsp.print("Memory used since last receive: ");
          hsp.println(memDiff);
          hsp.print("Total memory left: ");
          hsp.println(freeMem);
          memPct = ((freeMem / 262144.0) * 100.0);
          hsp.print(memPct); hsp.println("% memory left");
        }


        if (!SHORE) {               // if at sea, convey message verbatim
          hsp.println((char*)buf);
        }


        char tmp[numChars];
        for (int i = 0; i < numChars; i++) {
          tmp[i] = (char)buf[i];
        }
        char *token = strtok(tmp, " ");
        while (token != NULL) {
          if (strcmp(token, MODE) == 0) {
            debug_print("Request to change settings.");
            if (modeChangeRemote(buf) == 0) {
              debug_print("Transmission settings changed successfully.");
            }
          }
          if (strcmp(token, RCVD) == 0) {

            RH_RF95::ModemConfig new_modem_config = {
              set[0],
              set[1],
              0x0C  // LEAVE THIS ALONE
            };
            delay(100);
            driver.setModemRegisters(&new_modem_config);      // give it the configuration we specified
            delay(100);

            if (db) {
              driver.printRegisters();
            }

            hsp.println("confirmed. Mode has been changed to new settings.");
          }
          token = strtok(NULL, " ");
        }
      }
    }
  }
}
