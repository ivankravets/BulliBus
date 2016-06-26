
inline
unsigned short
crc_update( unsigned short crc, char c )
{
	int i;

	crc = crc ^ ((unsigned short)c << 8);
	for (i=0; i<8; i++)
	{
		if (crc & 0x8000)
			crc = (crc << 1) ^ 0x1021;
		else
			crc <<= 1;
	}

	return crc;
}
