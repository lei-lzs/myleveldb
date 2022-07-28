#include "log_reader.h"
#include "status.h"
#include "crc32c.h"
#include "coding.h"

namespace log {


bool Reader::ReadRecord(Slice* record, std::string* scratch)
{
	if (last_record_offset_ < initial_offset_) {

		// 制定了初始的offset
		if (!SkipToInitialBlock()) {
			return false;
		}
	}

	scratch->clear();
	record->clear();

	bool in_fragmented_record = false;

	uint64_t prospecttive_record_offset = 0;

	Slice fragment;

	while (true) {
		const unsigned int record_type = ReadPhysicalRecord(&fragment);

		uint64_t physical_record_offset = end_of_buffer_offset_ - buffer_.size() - kHeaderSize - fragment.size();

		//处理initial_offset_ 带来的残留
		if (resyncing_)
		{
			if (record_type == kMidType) {
				continue;
			}
			else if (record_type == kLastType) {
				resyncing_ = false;
				continue;
			}
			else {
				resyncing_ = false;
			}
		}



	}


}

//读一个物理记录
unsigned int Reader::ReadPhysicalRecord(Slice* result) {
	while (true) {
		if (buffer_.size() < kHeaderSize) {
			if (!eof_) {
				buffer_.clear();

				//从file_中读一个Block到buffer_
				Status status = file_->Read(kBlockSize, &buffer_, backing_store_);

				end_of_buffer_offset_ += buffer_.size();

				if (!status.ok()) {
					buffer_.clear();

					eof_ = true;

					return kEof;
				}
				else if (buffer_.size() < kBlockSize)
				{
					eof_ = true;
				}

				continue;
			}
			else
			{
				buffer_.clear();
				return kEof;
			}
		}

		//Parse the header
		const char* header = buffer_.data();

		//length
		const uint32_t a = static_cast<uint32_t>(header[4]) & 0xff;
		const uint32_t b = static_cast<uint32_t>(header[5]) & 0xff;
		const unsigned int type = header[6];
		const uint32_t length = a | (b << 8);

		if (kHeaderSize + length > buffer_.size()) //一个Record不可能跨Block
		{
			size_t drop_size = buffer_.size();
			buffer_.clear();

			if (!eof_) {
				ReportCorruption(drop_size, "bad record length");
				return kBadRecord;
			}

			return kEof;
		}

		//跳过长度为0的record
		if (type == kZeroType && length == 0)
		{
			buffer_.clear();
			return kBadRecord;
		}

		//crc校验
		if (checksum_)
		{
			uint32_t expected_crc = crc32c::Unmask(DecodeFixed32(header));
			uint32_t actual_crc = crc32c::Value(header + 6, 1 + length);
			if (actual_crc != expected_crc) {
				// Drop the rest of the buffer since "length" itself may have
				// been corrupted and if we trust it, we could find some
				// fragment of a real log record that just happens to look
				// like a valid log record.
				size_t drop_size = buffer_.size();
				buffer_.clear();
				ReportCorruption(drop_size, "checksum mismatch");
				return kBadRecord;
			}
		}

		// buffer_ 读取了一个record的长度
		buffer_.remove_prefix(kHeaderSize + length);

		// Skip physical record that started before initial_offset_
		if (end_of_buffer_offset_ - buffer_.size() - kHeaderSize - length <
			initial_offset_) {
			result->clear();
			return kBadRecord;
		}

		*result = Slice(header + kHeaderSize, length);
		return type;
	} // end while
} // end  ReadPhysicalRecord

} // end of namespace log;
