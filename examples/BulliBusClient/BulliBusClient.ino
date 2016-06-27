#include "BulliBus.h"

#define LED 13


Bulli bus( SER0 );
Passenger sensor( bus, "tmp1" );


volatile float temp = 12.34;

void handleCargo( Cargo &cargo ) {

	cargo.reply( temp );
}

void setup() {

	pinMode( LED, OUTPUT );

	// Not that only *some* baud rates are available on 8MHz AVR
	bus.begin( 19200 );

	sensor.onCargo( handleCargo );
}

void blink() {

	static bool on;

	on = !on;

	digitalWrite( LED, on );
}

void genTemp() {

	cli(); // protect against interrupt between writing value

	temp += 1.23; // measure somehow
	if( temp > 100.0 ) temp = 0;

	sei();

}

void loop() {

	genTemp();

	bus.delay( 250 );

	blink();
}
