#include <util/crc16.h>

#define crc_init() 0

#define crc_update( crc, c ) \
		_crc_xmodem_update( (crc), (c) );

