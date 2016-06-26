#include <util/crc16.h>

#define crc_update( crc, c ) \
		_crc_xmodem_update( (crc), (c) );

