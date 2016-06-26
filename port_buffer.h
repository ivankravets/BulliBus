class PortBuffer : public port_t {

public:
	PortBuffer( Buffer &buffer ) : buffer( buffer ) {}

	void init( int baud ) const {
	}
	bool clearToSend() const {
		return buffer.remaining() > 0;
	}
	void send( char ch ) const {
		buffer.put( ch );
	}
	bool dataAvailable() const {
		return buffer.remaining() > 0;
	}
	short receive() const {
		return buffer.get();
	}

private:
	Buffer &buffer;

};
