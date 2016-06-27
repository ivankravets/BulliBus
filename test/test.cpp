#include "../Buffer.h"
#include "../BulliBus.h"

#include "../port_buffer.h"

#include <cassert>
#include <iostream>
#include <cstdlib>
#include <cstring>

bool _assertEq( const char * msg, int is, int exp, int line  ) {

	if( is != exp ) {
		std::cerr << "WRONG: \"" << msg << "\"(" << line << ") -- is: " << is << " exp: " << exp << "\n";
		return false;
	} else {
		return true;
	}
}
bool _assertEqual( const char * msg, const char *exp, const char *is, int line ) {

	if( strcmp( exp, is ) != 0 ) {
		std::cerr << "WRONG: \"" << msg << "\"(" << line << ") -- is: " << is << " exp: " << exp << "\n";
		return false;
	} else {
		return true;
	} 
}

#define assertEq( msg, is, exp ) if( !_assertEq( (msg), (is), (exp), __LINE__ )) return;
#define assertEqual( msg, is, exp ) if( !_assertEqual( (msg), (is), (exp), __LINE__ )) return;

void testBufferCorrectUsage() {

	Buffer buffer;

	char out[ 5 ];
	int amount;

	assertEq( "init pos", buffer.position(), 0 );
	assertEq( "init lim", buffer.limit(), DEFAULT_BUFFER_SIZE );
	assertEq( "init rem", buffer.remaining(), DEFAULT_BUFFER_SIZE );

	amount = buffer.put( "test" );

	assertEq( "test pos", buffer.position(), 4 );
	assertEq( "test lim", buffer.limit(), DEFAULT_BUFFER_SIZE );
	assertEq( "test rem", buffer.remaining(), DEFAULT_BUFFER_SIZE-4 );

	buffer.flip();

	assertEq( "flip pos", buffer.position(), 0 );
	assertEq( "flip lim", buffer.limit(), 4 );
	assertEq( "flip rem", buffer.remaining(), 4 );

	amount = buffer.get( out, sizeof( out ) );

	assertEq( "get  amo", amount, 4 );
	assertEq( "get  pos", buffer.position(), 4 );
	assertEq( "get  lim", buffer.limit(), 4 );
	assertEq( "get  rem", buffer.remaining(), 0 );

	buffer.clear();

	assertEq( "clr  pos", buffer.position(), 0 );
	assertEq( "clr  lim", buffer.limit(), DEFAULT_BUFFER_SIZE );
	assertEq( "clr  rem", buffer.remaining(), DEFAULT_BUFFER_SIZE );
}

void testBufferMoreCorrectUsage() {

	Buffer buffer;

	char out[ 5 ];
	int amount;

	buffer.put( "1234" );
	assertEq( "pos", buffer.position(), 4 );

	buffer.put( 'A' );
	buffer.put( 'B' );
	assertEq( "pos2", buffer.position(), 6 );
	
}


void testBufferBorderConditions() {

	Buffer buffer;

	char out[ 20 ];
	unsigned int ch;
	int amount, i;

	assertEq( "init pos", buffer.position(), 0 );
	assertEq( "init lim", buffer.limit(), DEFAULT_BUFFER_SIZE );
	assertEq( "init rem", buffer.remaining(), DEFAULT_BUFFER_SIZE );

	amount = buffer.put( "1234567890123456789012345" );

	assertEq( "to long", amount, -1 );
	assertEq( "to long pos", buffer.position(), 0 );
	assertEq( "to long lim", buffer.limit(), DEFAULT_BUFFER_SIZE );
	assertEq( "to long rem", buffer.remaining(), DEFAULT_BUFFER_SIZE );

	amount = buffer.put( 'X' );

	assertEq( "one char", amount, 1 );
	assertEq( "one char pos", buffer.position(), 1 );
	assertEq( "one char lim", buffer.limit(), DEFAULT_BUFFER_SIZE );
	assertEq( "one char rem", buffer.remaining(), DEFAULT_BUFFER_SIZE-1 );

	amount = buffer.put( 'Y' );

	assertEq( "one char", amount, 1 );
	assertEq( "one char pos", buffer.position(), 2 );
	assertEq( "one char lim", buffer.limit(), DEFAULT_BUFFER_SIZE );
	assertEq( "one char rem", buffer.remaining(), DEFAULT_BUFFER_SIZE-2 );

	amount = buffer.put( "123456789AB" );

	assertEq( "one char", amount, 11 );
	assertEq( "one char pos", buffer.position(), 13 );
	assertEq( "one char lim", buffer.limit(), DEFAULT_BUFFER_SIZE );
	assertEq( "one char rem", buffer.remaining(), DEFAULT_BUFFER_SIZE-13 );

	buffer.flip();
	assertEq( "flip pos", buffer.position(), 0 );
	assertEq( "flip lim", buffer.limit(), 13 );
	assertEq( "flip rem", buffer.remaining(), 13 );

	ch = buffer.get();
	assertEq( "got char", ch, 'X' );
	assertEq( "flip pos", buffer.position(), 1 );
	assertEq( "flip lim", buffer.limit(), 13 );
	assertEq( "flip rem", buffer.remaining(), 13-1 );

	ch = buffer.get();
	assertEq( "got char", ch, 'Y' );
	assertEq( "get pos", buffer.position(), 2 );
	assertEq( "get lim", buffer.limit(), 13 );
	assertEq( "get rem", buffer.remaining(), 13-2 );

	for( i=2; i<13; i++ ) {
		ch = buffer.get();
	}
	assertEq( "got char", ch, 'B' );
	assertEq( "last pos", buffer.position(), 13 );
	assertEq( "last lim", buffer.limit(), 13 );
	assertEq( "last rem", buffer.remaining(), 0 );

	ch = buffer.get();
	assertEq( "got char", ch, -1 );
	assertEq( "ovfl pos", buffer.position(), 13 );
	assertEq( "ovfl lim", buffer.limit(), 13 );
	assertEq( "ovfl rem", buffer.remaining(), 0 );

	buffer.reset();

	amount = buffer.get( out, 20 );
	assertEq( "len", amount, 13 );
	assertEq( "ovfl pos", buffer.position(), 13 );
	assertEq( "ovfl lim", buffer.limit(), 13 );
	assertEq( "ovfl rem", buffer.remaining(), 0 );

}

void testBufferMoreBorderConditions() {

	Buffer buffer;

	char out[ 20 ];
	unsigned int ch;
	int amount, i;

	for( i=0; i<24; i++ ) {

		amount = buffer.put( 'A'+i );

		assertEq( "loop", amount, 1 );
		assertEq( "loop pos", buffer.position(), i+1 );
		assertEq( "loop lim", buffer.limit(), DEFAULT_BUFFER_SIZE );
		assertEq( "loop rem", buffer.remaining(), DEFAULT_BUFFER_SIZE-(i+1) );
	}
	assertEq( "left rem", buffer.remaining(), 0 );

	amount = buffer.put( 'A'+i );
	assertEq( "full", amount, -1 );
	assertEq( "full pos", buffer.position(), 24 );
	assertEq( "full lim", buffer.limit(), DEFAULT_BUFFER_SIZE );
	assertEq( "full rem", buffer.remaining(), 0 );
}

void testBufferEvenMoreBorderConditions() {

	Buffer buffer;

	char out[ 30 ];
	unsigned int ch;
	int amount, i;

	buffer.put( "1234567890" );
	assertEq( "init pos", buffer.position(), 10 );
	assertEq( "init lim", buffer.limit(), DEFAULT_BUFFER_SIZE );
	assertEq( "init rem", buffer.remaining(), 14 );

	buffer.flip();

	amount = buffer.get( out, sizeof( out ) );

	assertEq( "read", amount, 10 );

	buffer.reset();

	// we can repeat read because auf the reset
	amount = buffer.get( out, sizeof( out ) );

	assertEq( "again", amount, 10 );

	buffer.reset();

	// but we can't read it in a too small buffer
	// (we need an extra byte for NUL, so 10 is to small for a buffer of size 10.
	amount = buffer.get( out, 10 );

	assertEq( "too small", amount, -1 );
}

void testBufferCopy() {

	Buffer b1, b2;

	char out[30];
	int amount;

	b1.put( "1234" );
	assertEq( "init b1 pos", b1.position(), 4 );
	assertEq( "init b1 lim", b1.limit(), DEFAULT_BUFFER_SIZE );
	assertEq( "init b1 rem", b1.remaining(), 20 );

	b2.put( "ABCD" );
	assertEq( "init b2 pos", b2.position(), 4 );
	assertEq( "init b2 lim", b2.limit(), DEFAULT_BUFFER_SIZE );
	assertEq( "init b2 rem", b2.remaining(), 20 );

	b2.flip(); // !important

	b1 << b2;
	         
	assertEq( "combined pos", b1.position(), 8 );
	assertEq( "combined lim", b1.limit(), DEFAULT_BUFFER_SIZE );
	assertEq( "combined rem", b1.remaining(), 16 );

	b1.flip(); // important, too...

	amount = b1.get( out, sizeof( out ) );
	assertEq( "combined size", amount, 8 );
	assertEqual( "combined string", "1234ABCD", out );
}

int onCargoCalled = 0;
void onCargo( Cargo &cargo ) {

	//std::cout << "[onCargo]" << cargo.address << " / " << cargo.argv;

	assertEqual( "received addr", "tmp1", cargo.address );
	assertEqual( "argument", "get", cargo.argv );

	cargo.reply( "23.45" );

	onCargoCalled++;
}

void onReply( Cargo &cargo ) {

	//std::cout << "[onReply]" << cargo.address << " / " << cargo.argv;

	assertEqual( "received addr", "tmp1", cargo.address );
	assertEqual( "argument", "23.45", cargo.argv );

	onCargoCalled++;
}

void testBulli() {

	onCargoCalled = 0;

	char text[30];
	int amount;

	Buffer in, out;
	PortBuffer port( in, out );

	Bulli bus( port );
	Driver drv( bus );

	bus.begin( 9600 );

	// === request ===
	drv.request( "tmp1", "get", onReply );

	out.flip();

	amount = out.get( text, sizeof( text ) );

	assertEqual( "sent text", "tmp1 get~E62B\n", text );

	out.reset(); // Note that we can now read it again!

	// === get response ===
	Passenger passenger( bus, "tmp1" );
	passenger.onCargo( onCargo );

	in << out; // copy out to in

	in.flip();
	out.clear();

	bus.run();
	assertEq( "called", onCargoCalled, 1 );

	out.flip();
	amount = out.get( text, sizeof( text ) );
	assertEqual( "replied text", "tmp1>23.45~E755\n", text );

	in.clear();
	out.reset();
	in << out;
	in.flip();

	assertEq( "correct length", in.remaining(), amount );

	bus.run();

	assertEq( "called", onCargoCalled, 2 );
}

void testMoreBulli() {

	onCargoCalled = 0;

	char text[30];
	int amount;

	Buffer in, out;
	PortBuffer port( in, out );

	Bulli bus( port );
	Driver drv( bus );
	Passenger passenger( bus, "tmp1" );

	drv.send( "tmp1", "get" );

	out.flip();
	in << out;
	in.flip();
	out.clear();

	bus.run();

	assertEq( "got called", onCargoCalled, 1 );
}

unsigned long _millis = 1;
unsigned long millis(){ return _millis; }

#define RUN( what ) std::cout << #what ": "; (what)(); std::cout << "OK\n";

void on_error( const char * msg, Cargo &cargo ) {

	std::cout << ">>>ERROR: " << msg << " in '" << cargo.address << ": " << cargo.argv << "'\n";
}

int main( int argc, const char *argv[] ) {

	BulliBus::onError( on_error );

	RUN( testBufferCorrectUsage );
	RUN( testBufferMoreCorrectUsage );
	RUN( testBufferBorderConditions );
	RUN( testBufferMoreBorderConditions );
	RUN( testBufferEvenMoreBorderConditions );
	RUN( testBufferCopy );

	RUN( testBulli );
	//RUN( testMoreBulli );
	
	std::cout << "DONE\n";
}
