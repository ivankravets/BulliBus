
inline
ushort_t
crc_init()
{
	return 0;
}

__attribute__((optimize("unroll-loops")))
inline
ushort_t
crc_update( ushort_t crc, char c )
{
	register ushort_t i;

	crc = crc ^ ((unsigned short)c << 8);
	for (i=0; i<8; i++)
	{
		if (crc & 0x8000)
			crc = (crc << 1) ^ 0x1021;
		else
			crc <<= 1;
	}

	return crc & 0xFFFF; //!important for ushort_t
}
