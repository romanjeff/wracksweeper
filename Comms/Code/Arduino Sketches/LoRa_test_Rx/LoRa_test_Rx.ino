// This sketch is a pingback program to respond to a ping from the LoRa radio
// It sends back the exact message that it receives.

#include <SPI.h>
#include <RH_RF95.h>
#include <RHReliableDatagram.h>
#include <avr/dtostrf.h>
#include <string.h>  // for strstr() stringsearch

#define DEBUG


// choose which hardware serial port by toggling debug mode
#ifdef DEBUG
bool db = true;
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




//--------------------------LORA SETTINGS DEFS---------------------------------------------

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0
#define TIMEOUT 6000
#define RETRIES 5
#define SERVER_ADDRESS 1            // AUV radio is the server
#define CLIENT_ADDRESS 2            // shore pc is the client

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
RHReliableDatagram rf95(driver, SERVER_ADDRESS);    // rf95 is the msg manager

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

  RH_RF95::ModemConfig modem_config = {
    bw125cr45,
    sf128,
    0x0C  // LEAVE THIS ALONE
  };

  driver.setModemRegisters(&modem_config);

  //set to maximum possible output power
  driver.setTxPower(23, false);

  rf95.setRetries(RETRIES);
  rf95.setTimeout(TIMEOUT);
}




byte set[3];
const byte numChars = 255;
const char MODE[] = "-changemode\0";

void transmit(byte msg[], int msgLength) {
  if (!rf95.sendtoWait((uint8_t *)msg, msgLength + 1, CLIENT_ADDRESS)) {
    hsp.println("Transmission failed. No ACK received.");
  }
  else {
    hsp.println("Transmission successful. Bytes sent: ");
    for (int i = 0; i < msgLength; i++) {
      hsp.print(msg[i], HEX);
    }
    hsp.print("\n");
  }
}

int keyfromstring(char *key, int param)  // param = 0 for bw/cr, 1 for sf
{
  debug_print("the key i'm comparing is: ");
  debug_print(key)
  int NKEYS;
  if (param == 2) {
    NKEYS = N_sfKeys;
    for (int i = 0; i < NKEYS; i++) {
      if (strcmp(sfLUT[i].key, key) == 0) {
        debug_print("matched.")
        return sfLUT[i].val;
      }
    }
  }
  else {
    NKEYS = N_bwKeys;
    for (int i = 0; i < NKEYS; i++) {
      if (strcmp(bwLUT[i].key, key) == 0) {
        debug_print("matched.")
        return bwLUT[i].val;
      }
    }
  }
  return -1;
}



int modeChange(byte msg[]) {

  set[2] = 0;
  int sC = 0;
  hsp.print("The char array looks like: ");
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
    hsp.println("That didn't work.");
    hsp.println("Try again with the settings format -changemode bw___cr__ sf___");
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
  if (!rf95.sendtoWait((uint8_t *)confirmation, strlen(confirmation) + 1, CLIENT_ADDRESS)) {
    hsp.println("No Ack received, cannot change mode.");
    return -1;
  }
  else {
    hsp.println("Acknowledge received. Changing mode now.");

    RH_RF95::ModemConfig new_modem_config = {
      set[0],
      set[1],
      0x0C  // LEAVE THIS ALONE
    };
    delay(10);
    driver.setModemRegisters(&new_modem_config);      // give it the configuration we specified
    delay(10);
    if(db){driver.printRegisters();}

  }
  return 0;
}


int i = 0;

void loop()
{

  if (rf95.available())
  {
    // Should be a message for us now
    uint8_t buf[numChars];
    uint8_t len = sizeof(buf);
    uint8_t from;
    if (rf95.recvfromAck(buf, &len, &from)) {
      i++;
      if (i == 2) {
        hsp.println("Changing settings now.");
        RH_RF95::ModemConfig new_modem_config = {
          150,
          164,
          0x0C  // LEAVE THIS ALONE
        };
        delay(10);
        driver.setModemRegisters(&new_modem_config);      // give it the configuration we specified
        delay(10);
        if(db){driver.printRegisters();}
      }
      
      hsp.print("Message from client: "); hsp.println(from);
      hsp.println((char*)buf);
      for (int i = 0; i < len; i++) {
        hsp.print(buf[i], HEX);
      }
      hsp.print("\n");
      char tmp[numChars];
      for (int i = 0; i < numChars; i++) {
        tmp[i] = (char)buf[i];
      }
      char *token = strtok(tmp, " ");
      while ((token != NULL) && (token != "\n")) {
        if (strcmp(token, MODE) == 0) {
          debug_print("Request to change settings.");
          if (modeChange(buf) == 0) {
            hsp.println("Transmission settings changed successfully.");
          }
        }
        token = strtok(NULL, " ");
      }
    }

  }

}
