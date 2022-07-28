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
	,block_offset_(dest_length % kBlockSize) //��ʼ��ĳ��Block�е�ƫ��
{
}

log::Writer::~Writer()
{
}

//���һ��Slice��¼�� Slice�ĸ�ʽ����ô���ģ�
Status log::Writer::AddRecored(const Slice& slice)
{
	const char* ptr = slice.data();
	size_t left = slice.size();

	//���record̫����Ҫ�ֳɶ��fragment�����ڶ��block

	Status s;
	bool begin = true;
	
	// do while ���ٻ�ִ��һ��
	do {
		//��ǰBlock��ʣ�೤��
		const int leftover = kBlockSize - block_offset_; 
		assert(leftover >= 0);

		if (left < kHeaderSize) //���ʣ�೤��С��7��β����0
		{
			if (left > 0) {
				//C���ַ��������ַ���\0��(= ��\x00��) ��Ϊ������־����\0����һ��ASCII��Ϊ0���ַ���
				// �����������κο��ƶ�����Ҳ����һ������ʾ���ַ���
				//�ַ�����a��ʵ�ʰ���2���ַ�����a���͡�\0��C���ַ��������ַ���\0��(= ��\x00��) ��Ϊ������־��
				// ��\0����һ��ASCII��Ϊ0���ַ��������������κο��ƶ�����Ҳ����һ������ʾ���ַ���
				//�ַ�����a��ʵ�ʰ���2���ַ�����a���͡�\0��
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

		//����ǰ��fragmentд��Log
		s = EmitPhysicalRecord(type, ptr, fragment_length); 

		ptr += fragment_length;
		left -= fragment_length;
		begin = false;

	} while (s.ok() && left > 0);

	return s;
}

Status log::Writer::EmitPhysicalRecord(RecordType type, const char* ptr, size_t length)
{
	assert(length <= 0xffff); //header����2 byte��¼record���ȣ�����һ��record��󳤶�Ϊ0xffff
	assert(block_offset_ + kHeaderSize + length <= kBlockSize);

	//Format the header
	char buf[kHeaderSize];
	buf[4] = static_cast<char>(length & 0xff);
	buf[5] = static_cast<char> (length >> 8);
	buf[6] = static_cast<char>(type);

	//����crcУ����
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
	//����Blcok�е�ƫ��
	block_offset_ += kHeaderSize + length;

	return s;
}
