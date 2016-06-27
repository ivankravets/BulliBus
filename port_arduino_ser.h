class __SerNo : public port_t {
	
	void init( uint32_t baud ) const {
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
	short_t receive() const {
		return (short_t)__TheSerial.read();
	}
};
