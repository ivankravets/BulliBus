class PortBuffer : public port_t {

public:
	PortBuffer( Buffer &in, Buffer &out ) : in( in ), out( out ) {}

	void init( int baud ) const {
	}
	bool clearToSend() const {
		return out.remaining() > 0;
	}
	void send( char ch ) const {
		out.put( ch );
	}
	bool dataAvailable() const {
		return in.remaining() > 0;
	}
	short receive() const {
		return in.get();
	}

private:
	Buffer &in, &out;

};
