#ifndef BUFFER_H
#define BUFFER_H

#include "settings.h"

class Buffer {

	const int CAPACITY;
	int _limit;
	int _position;
	// int mark

public:
	Buffer();
	int put( char ch );
	int put( const char *msg );
	int get();
	int get( char *msg, int size );
	void clear();
	void flip();
	void reset();

	const char *plain();

	int limit() const;
	int position() const;
	int remaining() const;
	char next();

	Buffer& operator<<( Buffer &rhs );

private:
	char buffer[ BB_BUFFER_SIZE ];

};

#endif
