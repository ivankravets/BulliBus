#include "Buffer.h"

#include <string.h>
//#include <iostream>

Buffer::Buffer()
 : CAPACITY( DEFAULT_BUFFER_SIZE ) {

	clear();
}

int Buffer::put( char ch ) {

	if( !remaining() ) return -1;

	buffer[ _position++ ] = ch;

	return 1;
}

int Buffer::put( const char * msg ) {

	int size = strlen( msg ),
	             i;

	if( size > remaining() ) return -1;

	for( i=0; i<size; i++ ) {
		put( msg[ i ] );
	}

	return i;
}

int Buffer::get() {

	if( !remaining() ) return -1;

	return buffer[ _position++ ];
}

int Buffer::get( char *msg, int size ) {

	int i;

	if( size < remaining()+1 ) return -1;

	for( i=0; i<size; i++ ) {

		int ch = get();
		if( ch < 0 ) break;

		msg[ i ] = (char) ch;
	}

	msg[ i ] = '\0';

	return i;
}

void Buffer::clear() {
	_limit = CAPACITY;
	_position = 0;
}
void Buffer::flip() {
	_limit = _position;
	_position = 0;
}
void Buffer::reset() {
	_position = 0;
}
const char * Buffer::plain() {
	return buffer;
}

int Buffer::position() const {
	return _position;
}
int Buffer::limit() const {
	return _limit;
}
int Buffer::remaining() const {
	return _limit - _position;
}
char Buffer::next() {
	return buffer[ _position++ ];
}

Buffer& Buffer::operator<<( Buffer &rhs ) {

	while( remaining() && rhs.remaining() ) {

		put( rhs.get() );
	}

	return *this;
}
