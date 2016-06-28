#include "BulliBus.h"

//  PPP  A  Y  Y L    OO    A   DDD
//  PPP AAA  YY  L   O  O  AAA  D  D
//  P  A   A  Y  LLL  OO  A   A DDD

#define LED 13 // Mini Pro
//#define LED 2  // ESP12e


Bulli bus( SER0 );
Passenger clerk( bus, "clrk" );

void handleCargo( Cargo &cargo ) {

	// parse string and return 
	BulliBus::Args args( cargo );

	if( args.argc == 0 || String( args.argv[ 0 ] ) == "help" ) {

		cargo.reply( "HELP: clrk <arg1>, <arg2>, ... <arg8>" );

	} else {

		for( int i=0; i < args.argc; i++ ) {

			Serial.print( i );
			Serial.print( ": " );
			Serial.print( args.argv[ i ] );
			Serial.println();
		}

	}
}

void setup() {

	pinMode( LED, OUTPUT );
	digitalWrite( LED, true );

	clerk.onCargo( handleCargo );
	
	// Not that only *some* baud rates are available on 8MHz AVR
	bus.begin( 19200 );

}

void loop() {

	bus.delay( 250 ); // or some other arbitrary number
}
