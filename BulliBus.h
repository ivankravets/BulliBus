#ifndef BULLI_BUS_H
#define BULLI_BUS_H

#include "Arduino"

class Message {

	uint32_t address;
	int argc;
	const char * argv[];
};

typedef void (*callback)( Message *message );

class BulliBus {

	public:
		void init( int port );
		void signal( uint32_t address, const char * message );
		void request( uint32_t address, const char * message );

};

class BulliTrailer {

	public:
		void init( int port, uint32_t address );
		void onSignal( callback cb );
		void onRequest( callback cb );
		void reply( const char * message );
}

#endif
