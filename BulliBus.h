#ifndef BULLI_BUS_H
#define BULLI_BUS_H

#include "Buffer.h"

class port_t {
	friend class Bulli;
	virtual void init( int baud ) const = 0;
	virtual bool clearToSend() const = 0;
	virtual void send( char ch ) const = 0;
	virtual bool dataAvailable() const = 0;
	virtual short receive() const = 0;
};

#if defined( ARDUINO_ARCH_AVR )
	#pragma message "ARCH_AVR"
	#include "Arduino.h"
	#define BB_HAS_SERIAL0
	#define BB_CRC_AVR
	#define BB_8bit
#elif defined( ARDUINO_ARCH_ESP8266 )
	#pragma message "ARCH_ESP8266"
	#include "Arduino.h"
	#define BB_HAS_SERIAL0
	#define BB_HAS_SERIAL1
	#define BB_CRC_PLAIN
	#define BB_32bit
#elif defined( ENERGIA )
	#pragma message "ENERGIA"
	#include "Arduino.h"
	#define BB_HAS_SERIAL0
	#define BB_CRC_PLAIN
	#define BB_32bit
#else
	#pragma message "ARCH_x86"
	#include <stdio.h>
	#include <stdint.h>
	#include "ArduStub.h"
	#define BB_CRC_PLAIN
	#define BB_32bit
#endif

#ifdef BB_8bit
	typedef char char_t;
	typedef unsigned short ushort_t;
#else
	typedef int char_t;
	typedef unsigned int ushort_t;
#endif

#define ADDR( c ) \
	((c)[0]<<24 | (c)[1]<<16 | (c)[2]<<8 | (c)[3])

class Bulli;
class Driver;
class Passenger;
class Cargo;

typedef const char * bb_addr_t;
typedef void (*bb_callback_t)( Cargo &cargo );


#ifdef BB_HAS_SERIAL0
extern const port_t &SER0;
#endif
#ifdef BB_HAS_SERIAL1
extern const port_t &SER1;
#endif

// --- Settings ---
class BulliBus {

	public:
		static void onError( void (*cb_error)( const char *, Cargo& ) );
};

// --- Cargo ---
class Cargo {

	friend class Bulli;

	private:
		Bulli &bus;

		Cargo( Bulli &bus, bb_addr_t address, const char *argv );

	public:
		bb_addr_t address;
		const char * argv;

		void reply( const char * msg );
		void reply( String msg );

		template<typename T>
		void reply( T value );

};


// --- Bulli ---
class Bulli {

	friend class Cargo;
	friend class Driver;
	friend class Passenger;

private:
	const port_t &port;
	const Driver *driver;
	Passenger *passenger;

public:

	Bulli( const port_t &port );

	void begin( int speed );

	/*
	void withTimeouts( uint32_t initial_timeout_us, uint32_t consecutive_timeout_us );
	void withReceiveBuffer( char * buffer[], size_t size );
	void withTransmitBuffer( char * buffer, size_t size );
	*/

	void run();
	void delay( unsigned int ms );

private:

	void _tryReceive();
	void _trySend();
	void _processIn( Buffer buffer );
	void send( bb_addr_t addr, const char *msg, bool isreply );

	Buffer in;
	Buffer out;
};


// --- Driver ---
class Driver {

	friend class Bulli;

	private:
		Bulli &bus;
		bb_callback_t callback;
		bb_addr_t lastCall;

	public:
		Driver( Bulli &bus );

		void send( bb_addr_t address, const char * message ) const;
		void request( bb_addr_t address, const char * message, bb_callback_t callback );
};

// --- Passenger ---
class Passenger {

	friend class Bulli;

	private:
		Bulli &bus;
		Passenger *next;
		bb_callback_t callback;
		bb_addr_t address;

	public:

		Passenger( Bulli &bus, bb_addr_t address );

		void onCargo( bb_callback_t cb );
};
#endif
