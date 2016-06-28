#include "BulliBus.h"

//   * *    CC  L   I  EEE  N NN  TTT
//  *****  C    L   I  EE   NNNN   T
//   * *    CC  LLL I  EEE  NN N   T

#define LED 13 // Mini Pro
//#define LED 2  // ESP12e


Bulli bus( SER0 );
Passenger sensor( bus, "pin?" );

volatile float temp = 12.34;

//const char ON[] = "on", OFF[] = "off", TOG[] = "tog", GET[] = "get";

void handleCargo( Cargo &cargo ) {

	// just to see we got a message
	digitalWrite( LED, !digitalRead( LED ) );

	char no = cargo.address[ 3 ]; // 4th character of address
	int  pin = no-'0';            // convert to number. Only supports 0-9

	String value = String( cargo.payload );

	if( value == "on" ) {

		pinMode( pin, OUTPUT );
		digitalWrite( pin, true );

	} else if( value == "off" ) {

		pinMode( pin, OUTPUT );
		digitalWrite( pin, false );

	} else if( value == "tog" ) {

		digitalWrite( pin, !digitalRead( pin ) );

	} else if( value == "get" ) {
		
		cargo.reply( digitalRead( pin ) ? "on" : "off" );
	}

	// dont answer anything in case of set
}

void setup() {

	pinMode( LED, OUTPUT );
	digitalWrite( LED, true );

	sensor.onCargo( handleCargo );
	
	// Not that only *some* baud rates are available on 8MHz AVR
	bus.begin( 19200 );

}

void loop() {

	cli(); // protect against interrupt between writing value

	temp += 1.23; // measure somehow
	if( temp > 100.0 ) temp = 0;

	sei();

	bus.delay( 250 ); // or some other arbitrary number
}
