#include <stdio.h>

class Term : public port_t {

	void init( int baud ) const {
		printf( "Init: %dbaud\n", baud );
	}

	bool clearToSend() const {
		return true;
	}
	void send( char ch ) const {
		printf( "(%c)", ch );
	}
	bool dataAvailable() const {
		return false;//?
	}
	short receive() const {
		return getchar();
	}
};
