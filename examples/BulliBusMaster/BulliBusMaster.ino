#include "BulliBus.h"

//#define LED 13 // Mini Pro
#define LED 2  // ESP12e

Bulli bus( SER0 );
Driver driver( bus );

void onTmp( Cargo &cargo ) {

	digitalWrite( LED, false );

	Serial.print( "Got temp: " );
	Serial.println( cargo.argv );
}

void setup() {

	pinMode( LED, OUTPUT );

	// Not that only *some* baud rates are available on 8MHz AVR
	bus.begin( 19200 );
}

void loop() {

	digitalWrite( LED, true );

	driver.request( "tmp1", "get", onTmp );
	driver.request( "tmp2", "get", onTmp );

	// this delays loop but still handls bus messages
	bus.delay( 250 );
}
