#include <RF24Network.h>
#include <RF24.h> 
#include <nrf24_comms.h>
#include "Encoder.h"
#include <myPushButton.h>

//--------------------------------------------------------------------------------

/* DEV Board */
// #define SPI_CE        33    	// white/purple
// #define SPI_CS        26  	// green

/* M5Stack */
#define BUTTON_A_PIN 39
#define BUTTON_B_PIN 38
#define BUTTON_C_PIN 37

#define SPI_CE        5 	
#define SPI_CS        13	

RF24 radio(SPI_CE, SPI_CS);    // ce pin, cs pin
RF24Network network(radio); 

nrf24_comms nrf24;

unsigned long finishedSending = 0.0;

void packetAvailableCallback( uint16_t from ) {
  if (millis() - finishedSending > 20) {
    Serial.printf("Round trip time: %u (time idle: %u) \n", 
      millis() - nrf24.controllerPacket.id, 
      millis() - finishedSending);
  }
}

int joystickDeadZone = 3;
int joystickMin = 11;
int joystickMiddle = 127;
int joystickMax = 246;	

#define ENCODER_COUNTER_MIN	-18 	// decceleration (ie -20 divides 0-127 into 20)
#define ENCODER_COUNTER_MAX	12 		// acceleration (ie 15 divides 127-255 into 15)

byte mapEncoderToThrottle(int value) {
  return value > 0
    ? map(value, 0, ENCODER_COUNTER_MAX, 127, 255)
    : map(value, ENCODER_COUNTER_MIN, 0, 0, 127);
}

//--------------------------------------------------------------------------------

#define ENCODER_COUNTER_MIN	-18 	// decceleration (ie -20 divides 0-127 into 20)
#define ENCODER_COUNTER_MAX	12 		// acceleration (ie 15 divides 127-255 into 15)

void encoderChangedCallback( int value ) {
  // apply logic including deadman switch here
  nrf24.controllerPacket.throttle = mapEncoderToThrottle( value );
  Serial.printf("encoderChangedCallback: %d %d \n", value, nrf24.controllerPacket.throttle);
}

void encoderPushedCallback() {
  Serial.printf("Encoder button pushed!\n");
}

void encoderDoubleClickCallback() {
  Serial.printf("encoderDoubleClickCallback()\n");
}

bool encoderCanAccelerate() {
  return digitalRead( BUTTON_A_PIN ) == 0;
}


#define PULLUP		true
#define OFFSTATE	HIGH
void deadmanSwitchListener(int eventCode, int eventPin, int eventParam);
myPushButton deadmanSwitch(BUTTON_A_PIN, PULLUP, OFFSTATE, deadmanSwitchListener);
void deadmanSwitchListener(int eventCode, int eventPin, int eventParam) {
    
	if ( eventCode == deadmanSwitch.EV_RELEASED ) {
    updateEncoder(1);
  }
}

//--------------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);

  setupEncoder(
    ENCODER_COUNTER_MAX, 
    ENCODER_COUNTER_MIN, 
    encoderChangedCallback,
    encoderPushedCallback,
    encoderDoubleClickCallback,
    encoderCanAccelerate);

  SPI.begin();                                           // Bring up the RF network
  radio.begin();
  radio.setAutoAck(true);

  nrf24.begin(&radio, &network, /*role*/nrf24.RF24_CONTROLLER, /*callback*/packetAvailableCallback);
}

long now = 0;

#define SEND_PERIOD  500

void loop() {

  nrf24.update();

  deadmanSwitch.serviceEvents();

  updateEncoder();

  if ( millis() - now > SEND_PERIOD ) {
    now = millis();
    nrf24.controllerPacket.id = millis();
    if ( nrf24.sendPacketToBoard() ) {
      finishedSending = millis();
    }
    else {
      Serial.printf("Error sending to Board!\n");
    }
  }

  delay(100);
}
