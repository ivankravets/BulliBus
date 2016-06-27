#include "BulliBus.h"

#define LED 13

Bulli bus( SER0 );
Driver driver( bus );

void onTmp( Cargo &cargo ) {

	Serial.print( "Got temp: " );
	Serial.println( cargo.argv );
}

void setup() {

	bus.begin( 19200 );

	pinMode( LED, OUTPUT );

}

void blink() {

	static bool on;

	on = !on;

	digitalWrite( LED, on );
}

void loop() {

	driver.request( "tmp1", "get", onTmp );

	bus.delay( 250 );

	blink();
}
