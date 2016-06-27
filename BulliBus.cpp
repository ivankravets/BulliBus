#include "BulliBus.h"

#include <string.h>

#define DEFAULT_BUFFER_SIZE 24
#define DEFAULT_TIMEOUT 20

#define BB_WILDCARD '?'

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

static bool _matchAddress( const char * mine, const char * msg ) {

	register char_t i, m1, m2;

	for( i=0; i<4; i++ ) {

		m1 = mine[ i ];
		if( m1 == BB_WILDCARD ) continue;
		m2 = msg[ i ];
		if( m2 == BB_WILDCARD ) continue;
		if( m1 != m2 ) return false;
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
#endif

#ifdef BB_HAS_SERIAL1

	#define __TheSerial Serial1
	#define __SerNo Ser1

	#include "port_arduino_ser.h"

	const Ser1 _ser1;
	const port_t &SER1 = _ser1;
#endif

// === BulliBus ===

static void (* _cb_error)( const char *, Cargo& );

void BulliBus::onError( void (*cb_error)( const char *, Cargo& ) ) {

	_cb_error = cb_error;
}

// === Cargo ===

Cargo::Cargo( Bulli &bus, bb_addr_t address, const char *argv ) 
 : bus( bus ) {
	this->address = address;
	this->argv = argv;
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

// === Bulli ===
Bulli::Bulli( const port_t &port ) : 
		port( port )
{
	this->passenger = NULL;
	this->driver = NULL;
}

void Bulli::begin( uint32_t baud ) {
	port.init( baud );
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
		out.put( '>' );
		crc = crc_update( crc, '>' );
	} else {
		out.put( ' ' );
		crc = crc_update( crc, ' ' );
	}

	for( i=0; i<len; i++ ) {

		ch = msg[ i ];

		crc = crc_update( crc, ch );
		out.put( ch );
	}

	out.put( '~' );

	_putCrc( out, crc );

	out.put( '\n' );

	out.flip();
	
	_trySend();
}

void Bulli::run() {
	_trySend();
	_tryReceive();
}

void Bulli::delay( uint32_t ms ) {

	for( uint32_t end = millis() + ms; end > millis(); ) {

		run();
	}
}

void Bulli::_trySend() {

	while( port.clearToSend() && out.remaining() > 0 ) {

		port.send( out.next() );
	}
}

const char *  __findCrc( char * msg, int len ) {

	char *res = NULL;

	if( len >= 5 ) {

		char * ind = msg + len-1-4;

		if( *ind == '~' ) {

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

const char * __findPayload( char * msg, int len ) {

	char * result = &( msg[ 5 ] );

	while( *result == ' ' ) result++;

	return result;
}

void Bulli::_processIn( Buffer buffer ) {

	// note: this makes a copy of the buffer.
	// It may be possible to write this without a copy.
	char buf[ DEFAULT_BUFFER_SIZE+1 ]; // don't forget NUL

	buffer.flip();

	int len = buffer.get( buf, sizeof( buf ) );

	buffer.clear();
	// from this point we can start receiving again
	
	if( len > 5 ) {

		char type = buf[ 4 ];

		if( type == ' ' || type == '>' ) {

			bb_addr_t addr = __findAddress( buf, len );
			const char * payload = __findPayload( buf, len );
			Cargo cargo( *this, addr, payload );
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

			if( crcErr && _cb_error ) {

				_cb_error( "CRC", cargo );
			}

			// request
			if( type == ' ' ) {

				for( Passenger *p=passenger; p != NULL; p = p->next ) {

					if( _matchAddress( p->address, addr ) ) {

						if( crcErr ) {
							cargo.reply( "CRC ERR" );
							return;
						}

						if( p->callback != NULL )
								p->callback( cargo ); 
					}
				}
	
			// response
			} else if( type == '>' ) { //reponse

				if( driver && _matchAddress( driver->lastCall, addr ) ) {
				
					if( driver->lastCall ) {

						driver->callback( cargo );
					}
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

		if( ch == '\r' ) ch = '\n'; // treat \r and \n the same

		if( ch == '\n' ) {

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
	lastTime = 0;
}

void Driver::send( bb_addr_t address, const char *message ) const {

	bus.send( address, message, false );
}

void Driver::request( bb_addr_t address, const char *message, bb_callback_t cb ) {

	this->callback = cb;
	this->lastCall = address;

	unsigned long now = millis(),
	              timeout = now-lastTime;

	if( lastTime > 0 && timeout < DEFAULT_TIMEOUT )
			bus.delay( DEFAULT_TIMEOUT - timeout );

	send( address, message );

	lastTime = millis();
}


// === Passenger ===

Passenger::Passenger( Bulli &bus, bb_addr_t address )
 : bus( bus ) {

 	this->address = address;

	if( bus.passenger ) {
		this->next = bus.passenger->next;
	} else {
		this->next = NULL;
	}
	bus.passenger = this;
}

void Passenger::onCargo( bb_callback_t cb ) {

	this->callback = cb;
}
