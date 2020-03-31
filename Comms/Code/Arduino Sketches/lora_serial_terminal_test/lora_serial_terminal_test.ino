
// This sketch is a proof of concept to verify packet transmission of UART data 
// from one LoRa radio to another. It pings out serial input to a radio to a pingback
// program and reports the packet that it receives in response.
// Currently under construction to add the ability to send command to change transmit
// settings for datarate
 
#include <SPI.h>
#include <RH_RF95.h>

 
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
 
  Serial1.begin(115200);  // Feather M0 hardware UART tx/rx are tied to "Serial1" 
                        // and not Serial, use as with Serial library.
//  while (!Serial1) {
//    delay(1);
//  }
 
  delay(100);
 
  Serial.println("Feather LoRa TX Test!");
 
  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);
 
  while (!rf95.init()) {
    Serial1.println("LoRa radio init failed");
    Serial1.println("Uncomment '#define SERIAL_DEBUG' in RH_RF95.cpp for detailed debug info");
    while (1);
  }
  Serial1.println("LoRa radio init OK!");
 
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial1.println("setFrequency failed");
    while (1);
  }
  Serial1.print("Set Freq to: "); Serial.println(RF95_FREQ);
  
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  //rf95.setModemConfig(RH_RF95::Bw125Cr48Sf4096);

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);
}
 
int16_t packetnum = 0;  // packet counter, we increment per xmission
String payLoad;
int timeSent = 0;


// set vars for serial input function
const byte numChars = 255; //limits read to 255 bytes (full size of serial buffer
                           // and max length of a lora payload)
char receivedChars[numChars];
boolean newData = false;

void receiveWithStartAndEndMarkers(){
  static boolean receiveInprogress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while (Serial1.available() > 0 && newData == false) {
    rc = Serial.read();
    if(receiveInProgress == true){
      if (rc != endMarker){
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars -1;
        }
      }
      else {
        receivedChars[ndx] = '\0';
        receiveInProgress = false;
        ndx = 0;
        newData = true;
      }
    }
    else if(rc == startMarker){
      receiveInProgress = true;
    }
  }
}


void showNewData(){
  if(newData == true) {
    Serial1.print(receivedChars);
    newData = false;
  }
}

        }
      }
    }
  }
}




void loop()
{
  if(Serial1.available() == 0){
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.waitAvailableTimeout(1000)){ 
        // Should be a reply message for us now   
        if (rf95.recv(buf, &len)){
          Serial1.print((millis()-timeSent)/1000.0);
          Serial1.println(" seconds ping time.");
          Serial1.println((char*)buf);
          Serial1.print("RSSI: ");
          Serial1.println(rf95.lastRssi(), DEC);
        }
        
        else{
          Serial1.println("Receive failed.");
          timeSent = millis();
        }
        
      }
  }
  
  if(Serial1.available() > 0) {
    receiveWithStartAndEndMarkers();
    payLoad = receivedChars;
    // payLoad = Serial1.readStringUntil(/r);

    Serial1.print("Received ");
    Serial1.print(payLoad);
    if (payLoad == "setSlow"){
      rf95.setModemConfig(RH_RF95::Bw125Cr48Sf4096);
      Serial1.println("Spreading factor 12, CRC 4/8");
    }
 
    Serial1.println("Transmitting..."); // Send a message to rf95_server
  
    // make packet from float or string payload
    uint8_t packet[(payLoad.length() + 2)];
    for(int i = 0; i < payLoad.length(); i++){
      packet[i]=payLoad.charAt(i);
    }
    
    //char radiopacket[20] = "Hello World #      ";
    //itoa(packetnum++, radiopacket+13, 10);
   
    Serial1.print("Sending "); 
    Serial1.print(payLoad);
    packet[payLoad.length()+1] = 0;
    

    rf95.send(packet, sizeof(packet));
   
    Serial.println("Waiting for packet to complete..."); 

    rf95.waitPacketSent();
    timeSent = millis();
    Serial1.println("Waiting for reply...");
   

  }

}
