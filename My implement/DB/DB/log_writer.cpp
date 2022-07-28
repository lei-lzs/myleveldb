#include "log_writer.h"
#include "writable_file.h"
#include "coding.h"
#include "crc32c.h"


log::Writer::Writer(WritableFile* dest)
	:dest_(dest)
	,block_offset_ (0)
{

}

log::Writer::Writer(WritableFile* dest, uint64_t dest_length)
	:dest_(dest)
	,block_offset_(dest_length % kBlockSize) //初始在某个Block中的偏移
{
}

log::Writer::~Writer()
{
}

//添加一个Slice记录， Slice的格式是怎么样的？
Status log::Writer::AddRecored(const Slice& slice)
{
	const char* ptr = slice.data();
	size_t left = slice.size();

	//如果record太大，需要分成多个fragment，放在多个block

	Status s;
	bool begin = true;
	
	// do while 至少会执行一次
	do {
		//当前Block中剩余长度
		const int leftover = kBlockSize - block_offset_; 
		assert(leftover >= 0);

		if (left < kHeaderSize) //如果剩余长度小于7，尾部置0
		{
			if (left > 0) {
				//C的字符串中以字符’\0’(= ’\x00’) 作为结束标志，’\0’是一个ASCII码为0的字符，
				// 它不会引起任何控制动作，也不是一个可显示的字符。
				//字符串’a’实际包含2个字符：’a’和’\0’C的字符串中以字符’\0’(= ’\x00’) 作为结束标志，
				// ’\0’是一个ASCII码为0的字符，它不会引起任何控制动作，也不是一个可显示的字符。
				//字符串’a’实际包含2个字符：’a’和’\0’
				dest_->Append(Slice("\x00\x00\x00\x00\x00\x00",leftover));
			}

			block_offset_ = 0;
		}

		const size_t avail = kBlockSize - block_offset_ - kHeaderSize;
		const size_t fragment_length = (left < avail) ? left : avail; 

		RecordType type;
		const bool end = (fragment_length == left); //left < avail ,it's the end block, first or last
		if (begin && end) {
			type = kFullType;
		}
		else if (begin) {
			type = kFirstType;
		}
		else if (end) {
			type = kLastType;
		}
		else {
			type = kMidType;
		}

		//将当前的fragment写入Log
		s = EmitPhysicalRecord(type, ptr, fragment_length); 

		ptr += fragment_length;
		left -= fragment_length;
		begin = false;

	} while (s.ok() && left > 0);

	return s;
}

Status log::Writer::EmitPhysicalRecord(RecordType type, const char* ptr, size_t length)
{
	assert(length <= 0xffff); //header中用2 byte记录record长度，所以一个record最大长度为0xffff
	assert(block_offset_ + kHeaderSize + length <= kBlockSize);

	//Format the header
	char buf[kHeaderSize];
	buf[4] = static_cast<char>(length & 0xff);
	buf[5] = static_cast<char> (length >> 8);
	buf[6] = static_cast<char>(type);

	//计算crc校验码
	uint32_t crc = crc32c::Extend(type_crc_[type], ptr, length);
	crc = crc32c::Mask(crc);
	EncodeFixed32(buf, crc);

	//write the header and payload
	Status s = dest_->Append(Slice(buf, kHeaderSize));
	if (s.ok()) {
		s = dest_->Append(Slice(ptr, length));
		if (s.ok()) {
			s = dest_->Flush();
		}
	}
	//更新Blcok中的偏移
	block_offset_ += kHeaderSize + length;

	return s;
}
