#ifndef _LOG_FORMAT_HH_
#define _LOG_FORMAT_HH_


enum RecordType {
	kZeroType =0,
	kFullType,
	kFirstType,
	kMidType,
	kLastType
};

//常量才能在头文件定义
static const int kBlockSize = 32768; //uint16_t

static const int kMaxRecordType = 4;

//[ checksum(4 Byte)+length(2 Byte) + type(1 Byte)]
static const int kHeaderSize = 7;


#endif
