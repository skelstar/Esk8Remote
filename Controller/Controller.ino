#include <RF24Network.h>
#include <RF24.h> 
#include <nrf24_comms.h>

//--------------------------------------------------------------------------------

#define SPI_CE        33    	// white/purple
#define SPI_CS        26  	// green

RF24 radio(SPI_CE, SPI_CS);    // ce pin, cs pin
RF24Network network(radio); 

nrf24_comms nrf24;

unsigned long finishedSending = 0;

void packetAvailableCallback( uint16_t from ) {
    Serial.printf("Round trip time: %u (time idle: %u) \n", 
        millis() - nrf24.controllerPacket.id, 
        millis() - finishedSending);
}
//--------------------------------------------------------------------------------
void setup() {
    Serial.begin(9600);

	SPI.begin();                                           // Bring up the RF network
	radio.begin();
    radio.setAutoAck(true);

    nrf24.begin(&radio, &network, nrf24.RF24_CONTROLLER, packetAvailableCallback);
}

long now = 0;

#define SEND_PERIOD  200

void loop() {

    nrf24.update();

    if ( millis() - now > SEND_PERIOD ) {
        now = millis();
        nrf24.controllerPacket.id = millis();
        if ( nrf24.sendPacketToBoard() ) {
            finishedSending = millis();
            // Serial.printf("Sent %u to Board\n", nrf24.controllerPacket.id);
        }
        else {
            Serial.printf("Error sending to Board!\n");
        }
    }

    delay(1);
}
//--------------------------------------------------------------------------------
