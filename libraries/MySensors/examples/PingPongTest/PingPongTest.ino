/**
 * This sketch will play ping-pong with your gateway.
 * Gateway command : "pong"
 * 
 */ 

#include <MySigningNone.h>
//#include <MySigningAtsha204Soft.h>
#include <MyTransportNRF24.h>
#include <MyHwATMega328.h>
#include <MySensor.h>
#include <SPI.h>

#define LED_PIN  2  // Arduino Digital I/O pin number for first relay (second on pin+1 etc)

// NRFRF24L01 radio driver (set low transmit power by default) 
MyTransportNRF24 radio(RF24_CE_PIN, RF24_CS_PIN, RF24_PA_LEVEL_GW);  
// Message signing driver (none default)
MySigningNone signer;
// Select AtMega328 hardware profile
MyHwATMega328 hw;

// Change the soft_serial value to an arbitrary value for proper security
//MySigningAtsha204Soft signer(true);  // Select SW ATSHA signing backend

// Construct MySensors library
MySensor gw(radio, hw);
// Construct MySensors library with signing
//MySensor gw(radio, hw, signer);
const int NodeId = 22;  // CHANGE !!
MyMessage FlowMsg(0, V_VAR2);
long teller = 0;
boolean PING=false, toggle=true;

void setup()  
{   
  // Initialize library and add callback for incoming messages
  gw.begin(incomingMessage, NodeId, false);
  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("pingpong-node", "1.0");

   // Register all sensors to gw (they will be created as child devices)
   gw.present(0, S_CUSTOM);       
   gw.present(1, S_CUSTOM);
   // Then set relay pins in output mode
   pinMode(LED_PIN, OUTPUT);   
   digitalWrite(LED_PIN, toggle);
}

void loop() 
{
  if (PING == true) {
    teller++; 
    gw.send(FlowMsg.set(teller)); 
    PING=false;
    digitalWrite(LED_PIN, toggle);
    toggle = !toggle;
  }
  // Alway process incoming messages whenever possible
  gw.process();
}

void incomingMessage(const MyMessage &message) {
  // We only expect one type of message from controller. But we better check anyway.
  if (message.type==V_VAR1) 
     PING = true;
}

