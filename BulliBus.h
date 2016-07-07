/*****************************************************************************
 * BulliBus
 *
 * Library for easy serial bus communication
 *
 * see http://www.github.com/ScheintodX/BulliBus for help
 *
 * Author: Florian Bantner
 *
 * License: MIT
 */

#ifndef BULLI_BUS_H
#define BULLI_BUS_H

#include "Buffer.h"

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
	#define BB_HAS_SERIAL1
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
	typedef signed short short_t;
#else
	typedef int char_t;
	typedef unsigned int ushort_t;
	typedef signed int short_t;
#endif

#define ADDR( c ) \
	((c)[0]<<24 | (c)[1]<<16 | (c)[2]<<8 | (c)[3])

#define BB_NOPIN 9999

#include "settings.h"

class Bulli;
class Driver;
class Passenger;
class Cargo;

typedef const char * bb_addr_t;
typedef void (*bb_callback_t)( Cargo &cargo );

class port_t {
	friend class Bulli;
	virtual void init( uint32_t baud ) const = 0;
	virtual bool clearToSend() const = 0;
	virtual void send( char ch ) const = 0;
	virtual bool dataAvailable() const = 0;
	virtual void flush() const = 0;
	virtual short_t receive() const = 0;
};


#ifdef BB_HAS_SERIAL0
extern const port_t &SER0;
#endif
#ifdef BB_HAS_SERIAL1
extern const port_t &SER1;
#endif

// --- Settings / Utility Methods ---
namespace BulliBus {

	/**
	 * Install an error handler 
	 *
	 * This is e.g. called if there is an crc error
	 */
	static void onError( void (*cb_error)( const char *msg, Cargo& cargo ) );

	/**
	 * Helper for parsing args.
	 *
	 * This although supports quotes like in: 'tell herman "I'm home"'
	 *
	 * See example "BulliBusPayloadParsing.ino"
	 */
	class Args {

		char buffer[ BB_BUFFER_SIZE ];

		public:
			Args( Cargo &cargo );
			int argc;
			char * argv[ BB_MAX_ARGS ];

	};
};

/**
 * C A R G O
 *
 * Cargo encapsulates message send over the bus.
 *
 * Cargo can be passed along between driver and
 * one passanger at a time.
 *
 * If cargo is passed to a passenger he needs to be quick
 * to answer if he must.
 *
 * (This is not the best metaphor I've ever thought
 * of, but it should do.
 *
 * Write it in this video's comments if you can think
 * of a better one.)
 *
 */
class Cargo {

	friend class Bulli;
	friend class Args;

	private:
		Bulli &bus;

		Cargo( Bulli &bus, bb_addr_t address, char *payload );
		Cargo( Bulli &bus, bb_addr_t address );

	public:

		bool ok;
		bb_addr_t address;
		char *payload;

		void reply( const char *msg );
		void reply( String msg );

		template<typename T>
		void reply( T value );

};


/**
 * B U L L I
 *
 * Bulli represents the complete bus.
 * In fact it is the best bus ever built.
 *
 * You need one Bulli per application
 *
 * USAGE:
 *     decl:
 *         Bulli bus( SER0 );
 *     init:
 *         bus.begin( 19200 );
 *     loop:
 *         bus.run();
 */
class Bulli {

	friend class Cargo;
	friend class Driver;
	friend class Passenger;

private:
	const port_t &port;
	Driver *driver;
	Passenger *passenger;
	int txen_pin;

public:

	Bulli( const port_t &port, int txen_pin = BB_NOPIN );

	void begin( uint32_t speed );

	/*
	void withTimeouts( uint32_t initial_timeout_us, uint32_t consecutive_timeout_us );
	void withReceiveBuffer( char * buffer[], size_t size );
	void withTransmitBuffer( char * buffer, size_t size );
	*/

	void run();
	void delay( uint32_t ms );

private:

	void _txen( bool );

	void _tryReceive();
	void _trySend();

	void _processIn( Buffer buffer );

	void send( bb_addr_t addr, const char *msg, bool isreply );

	Buffer in;
	Buffer out;

};


/**
 * D R I V E R
 *
 * The driver knows which way to go. Some call him "Master".
 *
 * You can have a maximum of *one* Driver per Bus or you will
 * get in trouble with the cops.
 *
 * USAGE:
 *     decl:
 *         Driver otto( bus );
 *
 *     loop:
 *         otto.tell( "bart", "be quiet" );
 *         otto.request( "lisa", "Saxophone", onSaxophone );
 */
class Driver {

	friend class Bulli;

	private:
		Bulli &bus;
		bb_callback_t callback;
		bb_addr_t lastCall;
		uint32_t reservedUntil;
		void reserve( int millis );
		void release();

	public:
		Driver( Bulli &bus );

		void tell( bb_addr_t address, const char * message ) const;
		void request( bb_addr_t address, const char * message, bb_callback_t callback );
};

/**
 * P A S S E N G E R
 *
 * A passanger is only alowed to speak when the driver tells him to.
 * But don't call him slave.
 *
 * You can have as many Passenger on the bus as you like. But it can
 * get crowdy ...
 *
 * USAGE:
 *     decl:
 *         Passenger bart( "bart", handleBart ),
 *                   lisa( "lisa", handleLisa ),
 *                   todd( "todd", handleTodd ),
 *                   ralph( "ralf", handleRalf );
 *
 *     
 *         void handleBart( Cargo &cargo ) {
 *
 *              cargo.reply( "Kiss my butt" );
 *         }
 *
 *         void handleLisa( Cargo &cargo ) {
 *
 *              ...
 *         }
 */
class Passenger {

	friend class Bulli;

	private:
		Bulli &bus;
		Passenger *next;
		bb_callback_t callback;
		bb_addr_t address;
		uint32_t lastTime;

	public:

		Passenger( Bulli &bus, bb_addr_t address );

		void onCargo( bb_callback_t cb );
};
#endif
