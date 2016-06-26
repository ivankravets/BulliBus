class String {

	const char *str;

	String( const char * str ) {

		this->str = str;
	}

	const char * c_str() {

		return this->str;
	}
};
