#include <util/crc16.h>

#define crc_update( ch, crc ) \
		_crc_xmodem_update( (ch), (crc) );

