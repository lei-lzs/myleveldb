#ifndef _LEVELDB_OPTIONS_HH
#define _LEVELDB_OPTIONS_HH

class Comparator;

enum CompressionType {
	kNoCompression = 0x0,
	kSnappyCompression = 0x1
};

struct Options
{
	Options();
	const Comparator* comparator;

	bool create_if_missing = false;
};


#endif
