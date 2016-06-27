#include "BulliBus.h"

#define LED 13

Bulli bus( SER0 );
Driver driver( bus );

void onTmp( Cargo &cargo ) {

	if( (cargo.addr & 0x000f) == '1' ) {

		Serial.print( "Got temp: " );
		Serial.println( cargo.argv[ 0 ] );
	}
}

setup() {

	Serial.begin( 115200 );
	Serial.print( "Hello World\n" );

	bus.begin( 115200 );

	pinMode( LED, OUTPUT );

}

loop() {

	/*
	driver.request( "tmp1", "get", onTmp );

	bus.delay( 100 );
	*/
	delay( 100 );

	digitalWrite( LED, !digitalRead( LED ) );
}
