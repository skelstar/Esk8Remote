#ifndef nrf24_comms_h
#define nrf24_comms_h

#include <Arduino.h>
#include <RF24Network.h>
#include <RF24.h>


struct BoardStruct{
	bool vescOnline;
	float batteryVoltage;
	bool areMoving;
};

struct ControllerStruct {
	byte throttle;
	bool buttonC;
};

typedef void ( *PacketAvailableCallback )( uint16_t from );

class nrf24_comms
{
    public:

        enum Role {
			RF24_BOARD = 0,
			RF24_CONTROLLER	= 1
		};

        void begin(    
            RF24 *radio, 
            RF24Network *network, 
            Role role,
            PacketAvailableCallback packetAvailableCallback);
        void update();
        bool sendPacketToController();
        bool sendPacketToBoard();

//---------------------------------------------------------------------------------
bool esk8Lib::sendPacketToBoard() {

        
        BoardStruct boardPacket;
		ControllerStruct controllerPacket;

    private:

        RF24 *_radio;
		RF24Network *_network;
		Role _role;

        uint16_t readPacket();

        const uint16_t board_node = 00;
        const uint16_t controller_node = 01;
        const uint16_t passive_node = 02;
};