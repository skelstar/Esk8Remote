// #include <Arduino.h>
#include <vesc_comms.h>
#include <RF24Network.h>
#include <RF24.h> 
#include <nrf24_comms.h>

//--------------------------------------------------------------------------------

vesc_comms vesc;

struct VESC_DATA
{
  float batteryVoltage;
  float motorCurrent;
  float ampHours;
  float odometer;
};
VESC_DATA vescdata;

uint8_t vesc_packet[PACKET_MAX_LENGTH];

//--------------------------------------------------------------------------------
#define SPI_CE        33    // white/purple
#define SPI_CS        26  	// green

RF24 radio(SPI_CE, SPI_CS);    // ce pin, cs pin
RF24Network network(radio); 

nrf24_comms nrf24;

unsigned long firstPacketMillisDiff = 0;

void packetAvailableCallback( uint16_t from ) {
  nrf24.boardPacket.id = millis();

  vesc.setNunchuckValues(127, nrf24.controllerPacket.throttle, nrf24.controllerPacket.buttonC == true, 0);

  bool success = vesc.fetch_packet( vesc_packet ) > 0;
  if ( success ) {
    vescdata.batteryVoltage = vesc.get_voltage(vesc_packet);
    vescdata.motorCurrent = vesc.get_motor_current(vesc_packet);
    vescdata.ampHours = vesc.get_amphours_discharged(vesc_packet);
    Serial.printf("Batt: %.1f \n", vescdata.batteryVoltage);
  }
  else {
    Serial.printf("VESC not responding!\n");
  }
  
  nrf24.sendPacketToController();
}
//--------------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  Serial.printf("\nReady\n");

  vesc.init(115200);

  SPI.begin();                                           // Bring up the RF network
  radio.begin();
  radio.setAutoAck(true);

  nrf24.begin(&radio, &network, nrf24.RF24_BOARD, packetAvailableCallback);
}

long now = 0;

void loop() {

  nrf24.update();

  if ( millis() - now > 2000 ) {
    now = millis();
    int numbytes = vesc.fetch_packet( vesc_packet );
    if ( numbytes > 1 ) {
      vescdata.batteryVoltage = vesc.get_voltage(vesc_packet);
      vescdata.motorCurrent = vesc.get_motor_current(vesc_packet);
      vescdata.ampHours = vesc.get_amphours_discharged(vesc_packet);
      Serial.printf("Batt: %.1f \n", vescdata.batteryVoltage);
    }
    else {
      Serial.printf("VESC not responding!\n");
    }
  }
}