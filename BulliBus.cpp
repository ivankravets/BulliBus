#include "BulliBus.h"

#include <string.h>

#include <iostream>

#define DEFAULT_BUFFER_SIZE 24

char _i2c( uint8_t val ) {
	if( val <= 9 ) return '0' + val;
	else return 'A' + val-0xA;
}
uint8_t _c2i( char ch ) {

	if( ch >= '0' && ch <= '9' ) return ch-'0';
	if( ch >= 'A' && ch <= 'F' ) return ch-'A';
	if( ch >= 'a' && ch <= 'f' ) return ch-'A';
}

void _putCrc( Buffer &buf, unsigned short crc ) {

	buf.put( _i2c( (uint8_t)( (crc >> 12 )&0xF ) ) );
	buf.put( _i2c( (uint8_t)( (crc >>  8 )&0xF ) ) );
	buf.put( _i2c( (uint8_t)( (crc >>  4 )&0xF ) ) );
	buf.put( _i2c( (uint8_t)( (crc >>  0 )&0xF ) ) );
}
unsigned short _decodeCrc( const char * crc ) {

	unsigned short result;
	result = _c2i( crc[ 0 ] );
	result = result << 4 | _c2i( crc[ 1 ] );
	result = result << 4 | _c2i( crc[ 2 ] );
	result = result << 4 | _c2i( crc[ 3 ] );

	return result;
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

void Bulli::begin( int baud ) {
	port.init( baud );
}

void Bulli::send( bb_addr_t addr, const char *msg, bool isreply ) {

	int len = strlen( msg ), 
	    i;
	char ch;
	unsigned short crc = 0;

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

	//if( isreply ) out.put( '\n' );

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

void Bulli::delay( unsigned int ms ) {

	for( unsigned long end = millis() + ms; end < millis(); ) {

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

				int i;
				const char *p;

				unsigned short receivedCrc = _decodeCrc( crc ),
							   calculatedCrc = 0;

				for( i=0; i<4; i++ ) {
					calculatedCrc = crc_update( calculatedCrc, addr[ i ] );
				}
				calculatedCrc = crc_update( calculatedCrc, type );
				for( p=payload; p<crc-1; p++ ) {
					calculatedCrc = crc_update( calculatedCrc, *p );
				}

				crcErr = calculatedCrc != receivedCrc;

				if( crcErr ) {
					printf( "CRC: %04x/%04x\n", calculatedCrc, receivedCrc );
				}
			}

			if( crcErr && _cb_error ) {

				_cb_error( "CRC", cargo );
			}

			// request
			if( type == ' ' ) {

				for( Passenger *p=passenger; p != NULL; p = p->next ) {

					if( strncmp( p->address, addr, 4 ) == 0 ) {

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
				
				if( driver && driver->lastCall && strncmp( driver->lastCall, addr, 4 ) == 0 ) {

					if( crcErr ) {
						cargo.reply( "CRC ERR" );
						return;
					}

					driver->callback( cargo );
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

		int ch = port.receive();

		if( ch > 126 ) {
			in.reset();
			continue;
		}

		if( ch == '\r' ) ch = '\n';

		if( ch == '\n' ) {

			_processIn( in );
			continue;
		}

		in.put( ch );
	}
}


// === Driver ===

Driver::Driver( Bulli &bus ) 
 : bus( bus ) {

	bus.driver = this;
}

void Driver::send( bb_addr_t address, const char *message ) const {

	bus.send( address, message, false );
}

void Driver::request( bb_addr_t address, const char *message, bb_callback_t cb ) {

	this->callback = cb;
	this->lastCall = address;

	send( address, message );
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
