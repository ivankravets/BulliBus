#ifndef ARDUSTUB_H
#define ARDUSTUB_H

#include <string>
#include <sstream>

unsigned long millis();

class String {

private:
	std::string *_str;
public:

	String( const char * value ) {
		_str = new std::string( value );
	}
	String( float value ) {

		std::ostringstream ss;
		ss << value;
		_str = new std::string( ss.str() );
	}

	const char * c_str() {
		return _str->c_str();
	}
};

#endif
