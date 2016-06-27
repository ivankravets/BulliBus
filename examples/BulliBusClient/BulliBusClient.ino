#include "BulliBus.h"

//   CC  L   I  EEE  N NN  TTT
//  C    L   I  EE   NNNN   T
//   CC  LLL I  EEE  NN N   T

#define LED 13 // Mini Pro
//#define LED 2  // ESP12e


Bulli bus( SER0 );
Passenger sensor( bus, "tmp1" );


volatile float temp = 12.34;

void handleCargo( Cargo &cargo ) {

	static bool on;
	on = !on;

	digitalWrite( LED, on );

	cargo.reply( temp );
}

void setup() {

	pinMode( LED, OUTPUT );

	// Not that only *some* baud rates are available on 8MHz AVR
	bus.begin( 19200 );

	sensor.onCargo( handleCargo );
}

void loop() {

	cli(); // protect against interrupt between writing value

	temp += 1.23; // measure somehow
	if( temp > 100.0 ) temp = 0;

	sei();

	bus.delay( 250 ); // or some other arbitrary number
}
