class __SerNo : public port_t {
	
	void init( uint32_t baud ) const {
		__TheSerial.begin( baud );
	}
	bool clearToSend() const {
		#ifdef ENERGIA
			return true; // This may cause blocking
		#else
			return __TheSerial.availableForWrite();
		#endif
	}
	void send( char ch ) const {
		__TheSerial.write( ch );
	}
	bool dataAvailable() const {
		return __TheSerial.available();
	}
	short_t receive() const {
		return (short_t)__TheSerial.read();
	}
};
