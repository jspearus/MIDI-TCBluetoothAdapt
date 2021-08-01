


/*********************************************************************
 MIDI-TC pro Blutooth Adapter
*********************************************************************/

#include <Arduino.h>
#include <bluefruit.h>
#include <MIDI.h>
#include "SoftwareSerial.h"
#include <Wire.h>

#define SW_RXD    A0
#define SW_TXD    A1

// Declare an Software Serial instance
SoftwareSerial midiSerial(SW_RXD, SW_TXD);

MIDI_CREATE_INSTANCE(SoftwareSerial, midiSerial, MIDI);

BLEDis bledis;
BLEHidAdafruit blehid;

//Variables
String DataIn = "";
String x = "";
String y = "";
String xStep = "";
String yStep = "";

//Constants##################################
#define MOVE_STEP    10

//Function Declarations #############################
void SerialParser(String Com);
void home();
void moveCursor(int x, int y, int xStep, int yStep);
void leftClick();
void startAdv(void);
void printData();


void setup() 
{
  MIDI.begin(MIDI_CHANNEL_OMNI);
  Serial.begin(115200);
  while ( !Serial ) delay(10);   // for nrf52840 with native usb

  // Start the I2C Bus as Slave on address 9
  Wire.begin(9); 


  Serial.println("MIDI TC Bluetooth Adapter Ready..");

  Bluefruit.begin();
  // HID Device can have a min connection interval of 9*1.25 = 11.25 ms
  Bluefruit.Periph.setConnInterval(9, 16); // min = 9*1.25=11.25 ms, max = 16*1.25=20ms
  Bluefruit.setTxPower(4);    // Check bluefruit.h for supported values

  // Configure and Start Device Information Service
  bledis.setManufacturer("Adafruit Industries");
  bledis.setModel("Bluefruit Feather 52");
  bledis.begin();

  // BLE HID
  blehid.begin();

  // Set up and start advertising
  startAdv();
  delay(2000);
  home();
}

// MAIN LOOP ###########################################
void loop() 
{   
  while(Wire.available()){
    DataIn += (char)Wire.read(); // receive byte as a character
  }
  if(DataIn != ""){
    //Serial.println(DataIn);         // print Data Stream as string
    SerialParser(DataIn);
    DataIn = "";
  }
}

// END MAIN LOOP ########################################


// FUNCTIONS #########################################

void SerialParser(String Com) {
  x = Com.substring(0, Com.indexOf("@"));
  y = Com.substring(Com.indexOf("@") + 1, Com.indexOf("-"));
  xStep = Com.substring(Com.indexOf("-") + 1, Com.indexOf("&"));
  yStep = Com.substring(Com.indexOf("&") + 1, Com.indexOf("#"));

  //printData();
  moveCursor(x.toInt(), y.toInt(), xStep.toInt(), yStep.toInt());
  xStep = "";
}
//End SerialParser Function

void home(){
  int i;
    for (i=0; i<15; i++) {
     blehid.mouseMove(-127, -127);
   }
}

void moveCursor(int x, int y, int xStep, int yStep){
  home();
  for (int i=0; i<x; i++){    // move Horizontal - X AXIS
    blehid.mouseMove(xStep, 0);
  } 
  for (int i=0; i<y; i++) {   // Move Verticl - Y AXIS
    blehid.mouseMove(0, yStep);
  }
  delay(10);
  leftClick();
}

void leftClick(){
  blehid.mouseButtonPress(MOUSE_BUTTON_LEFT);
  delay(100);
  blehid.mouseButtonRelease();
  delay(10);
}

void startAdv(void)
{  
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_MOUSE);
  
  // Include BLE HID service
  Bluefruit.Advertising.addService(blehid);

  // There is enough room for 'Name' in the advertising packet
  Bluefruit.Advertising.addName();
  
  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html   
   */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
}

void printData(){
  if (xStep != "") {
    Serial.println(String("Xval=") + x + ", Yval=" + y + ", Xstep=" + xStep+ ", Ystep=" + yStep);
  } else {
    Serial.println("error ?");
  }
}
