#include "BulliBus.h"

//   CC  L   I  EEE  N NN  TTT
//  C    L   I  EE   NNNN   T
//   CC  LLL I  EEE  NN N   T

#define LED 13 // Mini Pro
//#define LED 2  // ESP12e


Bulli bus( SER0 );
Passenger sensor( bus, "tmp1" );
Passenger indicator( bus, "ind1" );

volatile float temp = 12.34;

void handleTemp( Cargo &cargo ) {

	cargo.reply( temp );
}

void handleIndicator( Cargo &cargo ) {

	String value( cargo.argv );

	if( value == "on" ) digitalWrite( LED, true );
	else if( value == "off" ) digitalWrite( LED, false );
	else if( value == "tog" ) digitalWrite( LED, !digitalRead( LED ) );
}

void setup() {

	Serial.begin( 19200 );

	pinMode( LED, OUTPUT );

	// Not that only *some* baud rates are available on 8MHz AVR
	bus.begin( 19200 );

	sensor.onCargo( handleTemp );
	indicator.onCargo( handleIndicator );
}

void loop() {

	cli(); // protect against interrupt between writing value

	temp += 1.23; // measure somehow
	if( temp > 100.0 ) temp = 0;

	sei();

	bus.delay( 250 ); // or some other arbitrary number
}
