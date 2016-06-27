#include "BulliBus.h"

//  MMMM   A    SSS  TTT  EEE  RRR
//  M  M  AAA   SS    T   EE   RRRR
//  M  M A   A SSS    T   EEE  R  R

//#define LED 13 // Mini Pro
//#define LED 2  // ESP12e
#define RED 30  // Energia
#define GRN 39  // Energia
#define BLU 40  // Energia

Bulli bus( SER1 );
Driver driver( bus );

void onTmp1( Cargo &cargo ) {

	Serial.print( "Got temp1: " );
	Serial.println( cargo.argv );

	digitalWrite( RED, false );
}

void onTmp2( Cargo &cargo ) {

	Serial.print( "Got temp2: " );
	Serial.println( cargo.argv );

	digitalWrite( GRN, false );
}

void onTmp3( Cargo &cargo ) {

	Serial.print( "Got temp3: " );
	Serial.println( cargo.argv );

	digitalWrite( BLU, false );
}

void setup() {

	pinMode( RED, OUTPUT );
	pinMode( GRN, OUTPUT );
	pinMode( BLU, OUTPUT );

	Serial.begin( 115200 );

	// Not that only *some* baud rates are available on 8MHz AVR
	bus.begin( 19200 );
}

void loop() {

	digitalWrite( RED, true );
	driver.request( "tmp1", "get", onTmp1 );

	digitalWrite( GRN, true );
	driver.request( "tmp2", "get", onTmp2 );

	digitalWrite( BLU, true );
	driver.request( "tmp3", "get", onTmp3 );

	// this delays loop but still handls bus messages
	bus.run();
}
