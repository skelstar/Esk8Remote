
// #include <EncoderBasicModule.h>
// #include <Rotary.h>
#include <Wire.h>
#include "i2cEncoderLibV1.h"


i2cEncoderLibV1 encoder(0x30); 

typedef void ( *ValueChangedCallback )( int value );
typedef void ( *ButtonPushedCallback )( void );
typedef bool ( *CanAccelerate )( void );
ValueChangedCallback _valueChangedCallback;
ButtonPushedCallback _buttonPushedCallback;
CanAccelerate _canAccelerate;

int counter = 0;

void setupEncoder(
  int max, 
  int min, 
  ValueChangedCallback valueChangedCallback,
  ButtonPushedCallback buttonPushedCallback,
  CanAccelerate canAccelerate) 
{
	Wire.begin();
	encoder.begin((INTE_ENABLE | LEDE_ENABLE | WRAP_DISABLE | DIRE_RIGHT | IPUP_ENABLE | RMOD_X1 ));
  encoder.writeCounter(0);
  encoder.writeMax(max); //Set maximum threshold
  encoder.writeMin(min); //Set minimum threshold
  _valueChangedCallback = valueChangedCallback;
  _buttonPushedCallback = buttonPushedCallback;
  _canAccelerate = canAccelerate;
}

void setEncoderCounter(int value) {
  counter = value;
}

void updateEncoder(bool force = false) {

	if (encoder.updateStatus() || force) {
		if (encoder.readStatus(E_PUSH)) {
      _buttonPushedCallback();
		}
		if (encoder.readStatus(E_MAXVALUE)) {
		}
		if (encoder.readStatus(E_MINVALUE)) {
		}

    int8_t newCounter = encoder.readCounterByte();
    bool accelerating = newCounter > counter;

    if ( newCounter > 0 ) {
      if ( accelerating && _canAccelerate() ) {
        counter = newCounter;
      }
      else {
        counter = 0;
        encoder.writeCounter(0);
      }
    }
    else {
      counter = newCounter;
    }
    _valueChangedCallback( counter );
	}
}