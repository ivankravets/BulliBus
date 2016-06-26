#include "BulliBus.h"

Bulli bus( SER1 );
Driver driver( bus );

void onTmp( Cargo &cargo ) {

	if( (cargo.addr & 0x000f) == '1' ) {

		Serial.print( "Got temp: " );
		Serial.println( cargo.argv[ 0 ] );
	}
}

setup() {

	Serial.begin( 115200 );

	bus.begin( 9600 );

}

loop() {

	driver.request( "tmp1", "get", onTmp );

	bus.delay( 100 );
}
