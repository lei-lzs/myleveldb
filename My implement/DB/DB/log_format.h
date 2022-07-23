#ifndef _LOG_FORMAT_HH_
#define _LOG_FORMAT_HH_


enum RecordType {
	kZeroType =0,
	kFirstType,
	kMidType,
	kLastType
};

static int kBlockSize = 32768; //uint16_t

static int kMaxRecordType = 4;

//[ checksum(4 Byte)+length(2 Byte) + type(1 Byte)]
static int kHeaderSize = 7;


#endif
