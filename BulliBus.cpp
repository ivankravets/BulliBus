#include "BulliBus.h"

#include <string.h>

#define BB_WILDCARD '?'
#define _REQ_ ' '
#define _RESP_ '>'
#define _CRC_ '~'
#define _NL_ '\n'

inline
char_t _i2c( char_t val ) {
	if( val <= 9 ) return '0' + val;
	else return 'A' + val-0xA;
}
inline
static char_t _c2i( char_t ch ) {

	if( ch >= '0' && ch <= '9' ) return ch-'0';
	if( ch >= 'A' && ch <= 'F' ) return ch-'A'+10;
	if( ch >= 'a' && ch <= 'f' ) return ch-'a'+10;
	return 0;
}
static void _putCrc( Buffer &buf, ushort_t crc ) {

	buf.put( _i2c( (char)( (crc >> 12 )&0xF ) ) );
	buf.put( _i2c( (char)( (crc >>  8 )&0xF ) ) );
	buf.put( _i2c( (char)( (crc >>  4 )&0xF ) ) );
	buf.put( _i2c( (char)( (crc >>  0 )&0xF ) ) );
}
static ushort_t _decodeCrc( const char * crc ) {

	ushort_t result;
	result = _c2i( crc[ 0 ] );
	result = (result << 4) | _c2i( crc[ 1 ] );
	result = (result << 4) | _c2i( crc[ 2 ] );
	result = (result << 4) | _c2i( crc[ 3 ] );

	return result;
}

static bool _matchMineAddressTo( bb_addr_t mine, bb_addr_t other ) {

	register char_t i, m, o;

	for( i=0; i<4; i++ ) {

		m = mine[ i ];
		if( m == BB_WILDCARD ) continue;
		o = other[ i ];
		if( o == BB_WILDCARD ) continue;
		if( m != o ) return false;
	}
	return true;
}

// === CRC ===
#ifdef BB_CRC_AVR
	#include "crc_avr.h"
#elif defined BB_CRC_PLAIN
	#include "crc_plain.h"
#endif

// === PORTS ===
#ifdef BB_HAS_SERIAL0

	#define __TheSerial Serial
	#define __SerNo Ser0

	#include "port_arduino_ser.h"

	const Ser0 _ser0;
	const port_t &SER0 = _ser0;

	#undef __TheSerial
	#undef __SerNo

#endif

#ifdef BB_HAS_SERIAL1

	#define __TheSerial Serial1
	#define __SerNo Ser1

	#include "port_arduino_ser.h"

	const Ser1 _ser1;
	const port_t &SER1 = _ser1;

	#undef __TheSerial
	#undef __SerNo
#endif

// === BulliBus ===

// Error callback
static void (* _cb_error)( const char *, Cargo& );

void BulliBus::onError( void (*cb_error)( const char *, Cargo& ) ) {

	_cb_error = cb_error;
}

// Args helper
BulliBus::Args::Args( Cargo &cargo ) {

	if( cargo.payload == NULL ) {
		argv[ 0 ] = 0;
		argc = 0;
		return;
	}

	register char_t c = 0;
	register char * p = cargo.payload;

	bool in = false;

	while( true ) {

		while( *p == ' ' ) p++; // skip spaces

		if( *p == '"' ){ in=true; p++; };

		if( *p == '\0' ) break; // this skips " as last char, too. ok.

		argv[ c++ ] = p;

		while( (in || *p != ' ') && *p != '\0' && *p != '"' ) p++; // skip text

		if( *p == '\0' ) break;

		*p++ = '\0';

		if( c == BB_MAX_ARGS ) break;
	}

	argc = c;
}

// === Cargo ===

Cargo::Cargo( Bulli &bus, bb_addr_t address, char *payload ) 
 : bus( bus ) {
 	this->ok = true;
	this->address = address;
	this->payload = payload;
}
Cargo::Cargo( Bulli &bus, bb_addr_t address )
 : bus( bus ) {
	this->ok = false;
	this->address = address;
	this->payload = NULL;
}
void Cargo::reply( const char *msg ) {
	bus.send( address, msg, true );
}
void Cargo::reply( String msg ) {
	reply( msg.c_str() );
}
template<typename T>
void Cargo::reply( T value ) {
	reply( String( value ) );
}
template void Cargo::reply<int>( int value );
template void Cargo::reply<long>( long value );
template void Cargo::reply<float>( float value );
template void Cargo::reply<double>( double value );

void Bulli::_txen( bool en ) {

	if( txen_pin != BB_NOPIN ) {
		if( txen_pin < 0 ) { 
			digitalWrite( -txen_pin, !en );
		} else {
			digitalWrite( txen_pin, en );
		}
	}
}


// === Bulli ===
Bulli::Bulli( const port_t &port, int txen_pin ) : 
		port( port ) {

	this->txen_pin = txen_pin;
	this->passenger = NULL;
	this->driver = NULL;
}

void Bulli::begin( uint32_t baud ) {

	port.init( baud );

	if( txen_pin != BB_NOPIN ) {
		_txen( false );
		pinMode( txen_pin < 0 ? -txen_pin : txen_pin, OUTPUT );
	}

}

void Bulli::send( bb_addr_t addr, const char *msg, bool isreply ) {

	int len = strlen( msg );
	register char_t i;
	register char_t ch;
	register ushort_t crc = crc_init();

	out.clear();

	for( i=0; i<4; i++ ) {

		ch = addr[ i ];

		crc = crc_update( crc, ch );
		out.put( ch );
	}

	if( isreply ) {
		out.put( _RESP_ );
		crc = crc_update( crc, _RESP_ );
	} else {
		out.put( _REQ_ );
		crc = crc_update( crc, _REQ_ );
	}

	for( i=0; i<len; i++ ) {

		ch = msg[ i ];

		crc = crc_update( crc, ch );
		out.put( ch );
	}

	out.put( _CRC_ );

	_putCrc( out, crc );

	out.put( _NL_ );

	out.flip();
	
	_trySend();
}

void Bulli::run() {

	//_trySend(); // not needed because we block sending
	_tryReceive();

	//Serial.print( "." );
	// notify driver of failed request
	if( driver && driver->lastCall && driver->reservedUntil < millis() ) {
		Cargo cargo = Cargo( *this, driver->lastCall );
		driver->callback( cargo );
		Serial.println( "err" );
		driver->release();
	}
}

void Bulli::delay( uint32_t ms ) {

	for( uint32_t end = millis() + ms; end > millis(); ) {

		run();
	}
}

void Bulli::_trySend() {

	_txen( true );

	while( out.remaining() > 0 ) {
		port.send( out.next() );
	}
	port.flush(); //wait for sending complete

	_txen( false );

	/*
	while( port.clearToSend() && out.remaining() > 0 ) {

		port.send( out.next() );
	}
	*/
}

char *  __findCrc( char * msg, int len ) {

	char *res = NULL;

	if( len >= 5 ) {

		char * ind = msg + len-1-4;

		if( *ind == _CRC_ ) {

			*ind = '\0';
			res = ind+1;
		}
	}
	return res;
}

bb_addr_t __findAddress( char * msg, int len ) {

	msg[ 4 ] = '\0';
	return msg;
}

char * __findPayload( char * msg, int len ) {

	if( len < 6 ) return NULL;

	char * result = &( msg[ 5 ] );

	while( *result == ' ' ) result++; // strip leading whitespace

	return result;
}

void Bulli::_processIn( Buffer buffer ) {

	// note: this makes a copy of the buffer.
	// It may be possible to write this without a copy.
	char buf[ BB_BUFFER_SIZE+1 ]; // don't forget NUL

	buffer.flip();

	int len = buffer.get( buf, sizeof( buf ) );

	buffer.clear();
	// from this point we can start receiving again
	
	if( len > 3 ) {

		char type = len > 4 ? buf[ 4 ] : _REQ_;

		if( type == _REQ_ || type == _RESP_ ) {

			bb_addr_t addr = __findAddress( buf, len );
			char * payload = __findPayload( buf, len );
			const char * crc = __findCrc( buf, len );
			bool crcErr = false;

			if( crc != NULL ) {

				char_t i;
				register const char *p;

				ushort_t receivedCrc = _decodeCrc( crc ),
				         calculatedCrc = crc_init();

				for( i=0; i<4; i++ ) {
					calculatedCrc = crc_update( calculatedCrc, addr[ i ] );
				}
				calculatedCrc = crc_update( calculatedCrc, type );
				for( p=payload; p<crc-1; p++ ) {
					calculatedCrc = crc_update( calculatedCrc, *p );
				}

				crcErr = calculatedCrc != receivedCrc;
			}

			Cargo cargo( *this, addr, payload );

			if( crcErr ) {
				if( _cb_error ) _cb_error( "CRC", cargo );
				return;
			}

			// request
			if( type == ' ' ) {

				Passenger *p = passenger;
				for( ; p != NULL; p = p->next ) {

					if( _matchMineAddressTo( p->address, addr ) ) {

						if( p->callback != NULL )
								p->callback( cargo ); 
					}
				}
	
			// response
			} else if( type == _RESP_ ) { //reponse

				if( driver && driver->lastCall && _matchMineAddressTo( driver->lastCall, addr ) ) {
				

					driver->callback( cargo );
					driver->release();
					Serial.println( "ok" );
				}

			}

		} else {
			//just ignore
		}
	} else {
		//just ignore
	}
}

void Bulli::_tryReceive() {

	while( port.dataAvailable() && in.remaining() > 0 ) {

		short_t ch = port.receive();

		if( driver ) driver->reserve( BB_TIMEOUT );

		if( ch == '\r' ) ch = _NL_; // treat \r and \n the same

		if( ch == _NL_ ) {

			_processIn( in );
			in.clear();
			continue;
		}

		if( (ch > 0x7E) || (ch < 0x20) ) { // every unknown character resets
			printf( "RESET" );
			in.clear();
			continue;
		}

		in.put( ch );
	}
}


// === Driver ===

Driver::Driver( Bulli &bus ) 
 : bus( bus ) {

	bus.driver = this;
	//release();
	lastCall = NULL;
	reserve( 0 );
}

void Driver::tell( bb_addr_t address, const char *message ) const {

	bus.send( address, message, false );
}

void Driver::request( bb_addr_t address, const char *message, bb_callback_t cb ) {

	// wait until reservation gets freed.
	while( millis() < reservedUntil ) bus.run();

	// after timeout reached
	this->callback = cb;
	this->lastCall = address;

	tell( address, message );

	reserve( BB_TIMEOUT );
}

void Driver::reserve( int ms ) {

	reservedUntil = millis() + ms;
}

void Driver::release() {
	lastCall = NULL;
	reserve( 0 );
	Serial.println( "release" );
}


// === Passenger ===

Passenger::Passenger( Bulli &bus, bb_addr_t address )
 : bus( bus ) {

 	this->address = address;

	if( bus.passenger ) {
		this->next = bus.passenger;
	} else {
		this->next = NULL;
	}
	bus.passenger = this;
}

void Passenger::onCargo( bb_callback_t cb ) {

	this->callback = cb;
}
