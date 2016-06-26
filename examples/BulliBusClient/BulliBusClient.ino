#include "BulliBus.h"

Bulli bus( SER0 );
Passenger sensor( bus, "tmp1" );

float temp;

void handleCargo( Cargo &cargo ) {

	// todo: only one parameter?
	
	cargo.reply( temp );
}

void setup() {

	bus.begin( 9600 );

	sensor.onCargo( handleCargo );
}

void loop() {

	cli();
	temp = 12.3; // measure somehow
	sei();

	bus.delay( 100 );
}
