class __SerNo : public port_t {
	
	void init( int baud ) const {
		__TheSerial.begin( baud );
	}
	bool clearToSend() const {
		return __TheSerial.availableForWrite();
	}
	void send( char ch ) const {
		__TheSerial.write( ch );
	}
	bool dataAvailable() const {
		__TheSerial.available();
	}
	short receive() const {
		return (short)__TheSerial.read();
	}
};
