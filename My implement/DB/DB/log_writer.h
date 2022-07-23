#ifndef _LEVELDB_LOG_WRITER_HH
#define  _LEVELDB_LOG_WRITER_HH

#include "status.h"
#include "log_format.h"
#include <cstdint>

class WritableFile;
class Slice;

namespace log {
	
	class Writer {
	public:
		explicit Writer(WritableFile* dest);

		Writer(WritableFile* dest, uint64_t dest_length);

		Writer(const Writer&) = delete;

		Writer& operator =(const Writer&) = delete;

		~Writer();

		Status AddRecored(const Slice& slice);

	private:
		Status EmitPhysicalRecord(RecordType type, const char* ptr, size_t length);

		WritableFile* dest_;
		int block_offset_; //文件当前偏移

		uint32_t type_crc_[kMaxRecordType + 1]; // RecordType 的crc值
	};
}


#endif
