
#include <Wire.h>
#include <i2cEncoderLibV2.h> // https://github.com/skelstar/ArduinoDuPPaLib/blob/master/examples/I2CEncoderV2/Basic/Basic.ino


i2cEncoderLibV2 encoder(0x01); 

typedef void ( *ValueChangedCallback )( int value );
typedef void ( *ButtonPushedCallback )( void );
typedef void ( *DoubleClickCallback )( void );
typedef bool ( *CanAccelerate )( void );
ValueChangedCallback _valueChangedCallback;
ButtonPushedCallback _buttonPushedCallback;
DoubleClickCallback _doubleClickCallback;
CanAccelerate _canAccelerate;

int counter = 0;

void setupEncoder(
  int max, 
  int min, 
  ValueChangedCallback valueChangedCallback,
  ButtonPushedCallback buttonPushedCallback,
  DoubleClickCallback doubleClickCallback,
  CanAccelerate canAccelerate) 
{
	Wire.begin();
  encoder.reset();
	encoder.begin(
    i2cEncoderLibV2::INT_DATA | 
    i2cEncoderLibV2::WRAP_DISABLE |
    i2cEncoderLibV2::DIRE_LEFT | 
    i2cEncoderLibV2::IPUP_ENABLE |
    i2cEncoderLibV2::RMOD_X1 | 
    i2cEncoderLibV2::STD_ENCODER);
  encoder.writeCounter((int32_t) 0); /* Reset the counter value */
  encoder.writeMax((int32_t) max); /* Set the maximum threshold*/
  encoder.writeMin((int32_t) min); /* Set the minimum threshold */
  encoder.writeStep((int32_t) 1); /* Set the step to 1*/
  encoder.writeInterruptConfig(0xff); /* Enable all the interrupt */
  encoder.writeAntibouncingPeriod(20); /* Set an anti-bouncing of 200ms */
  encoder.writeDoublePushPeriod(50); /*Set a period for the double push of 500ms */  
  _valueChangedCallback = valueChangedCallback;
  _buttonPushedCallback = buttonPushedCallback;
  _doubleClickCallback = doubleClickCallback;
  _canAccelerate = canAccelerate;
}

void setEncoderCounter(int value) {
  counter = value;
}

void updateEncoder(bool force = false) {

  if (encoder.updateStatus() || force) {
    if (encoder.readStatus(i2cEncoderLibV2::PUSHR)) {
      // released
    }
    if (encoder.readStatus(i2cEncoderLibV2::PUSHP)) {
      // pushed
      _buttonPushedCallback();
    }
    if (encoder.readStatus(i2cEncoderLibV2::PUSHD)) {
      _doubleClickCallback();
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