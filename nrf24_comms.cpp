#include "nrf24_comms.h"
#include <Arduino.h>

void nrf24_comms::begin(
    RF24 *radio, 
    RF24Network *network, 
    Role role,
    PacketAvailableCallback packetAvailableCallback) 
{
    _role = role;
	_radio = radio;
	_network = network;
	_packetAvailableCallback = packetAvailableCallback;

	_radio->begin();
	_radio->setPALevel(RF24_PA_MAX);
	_radio->setDataRate(RF24_1MBPS);

    switch (_role) {
        case RF24_BOARD:
            network->begin(/*channel*/ 90, /*node address*/ board_node);
            break;
        case RF24_CONTROLLER:
            network->begin(/*channel*/ 90, /*node address*/ controller_node);
            break;
    }
    _radio->printDetails();                   // Dump the configuration of the rf unit for debugging
}
//---------------------------------------------------------------------------------
void nrf24_comms::update() {
	_network->update();
	if ( _network->available() ) {
		uint16_t from = readPacket();
		_packetAvailableCallback( from );
	}
}
//---------------------------------------------------------------------------------
// reads packet into struct and returns node that packet came from
uint16_t nrf24_comms::readPacket() {

	RF24NetworkHeader header;                            // If so, take a look at it
    _network->peek(header);

	if ( header.from_node == RF24_CONTROLLER ) {
		 _network->read(header, &controllerPacket, sizeof(controllerPacket));
	}
	else if ( header.from_node == RF24_BOARD ) {
		_network->read(header, &boardPacket, sizeof(boardPacket));
	}
	else {
		Serial.printf("ERROR CONDITION!!! readPacket (from_node: '%d') (23) \n", header.from_node);
	}
	return header.from_node;
}
//---------------------------------------------------------------------------------
bool esk8Lib::sendPacketToController() {
	RF24NetworkHeader header( RF24_CONTROLLER );
	return _network->write(header, &boardPacket, sizeof(BoardStruct));
}
//---------------------------------------------------------------------------------
bool esk8Lib::sendPacketToBoard() {
	RF24NetworkHeader header( RF24_BOARD );
	controllerPacket.id++;
	return _network->write(header, &controllerPacket, sizeof(ControllerStruct));
}
//---------------------------------------------------------------------------------
