#include "format.h"
#include "coding.h"
#include "options.h"

void BlockHandle::EncodeTo(std::string* dst) const
{
	PutVarint64(dst, offset_);
	PutVarint64(dst, size_);
}

Status BlockHandle::DecodedFrom(Slice* input)
{
	if (GetVarint64(input, offset_) && GetVarint64(input, size_)) {
		return Status::OK();
	}
	return Status::Corruption("bad block handle");
}

void Footer::EncodeTo(std::string* dst) const
{
	// 40 + magic number (8)
	metaindex_handle_.EncodeTo(dst);
	index_handle_.EncodeTo(dst);

	//uint32_t len = VarintLength(metaindex_handle_.offset()) + VarintLength(metaindex_handle_.size())
	//	+ VarintLength(index_handle_.offset()) + VarintLength(index_handle_.size());
	uint32_t len = dst->size();
	uint32_t panding = 40 - len;
	
	dst->append(panding,' ');

	PutFixed32(dst, static_cast<uint32_t>(kTableMagicNumber & 0xffffffffu));
	PutFixed32(dst, static_cast<uint32_t>(kTableMagicNumber >> 32));

}

Status Footer::DecodedFrom(Slice* input)
{
	const char* magic_ptr = input->data() + kEncodedLength - 8;
	const uint32_t magic_lo = DecodeFixed32(magic_ptr);
	const uint32_t magic_hi = DecodeFixed32(magic_ptr + 4);

	const uint64_t magic = static_cast<uint64_t>(magic_hi) << 32 |
		static_cast<uint64_t>(magic_lo);

	if (magic != kTableMagicNumber) {
		return Status::Corruption("not an sstable (bad magic number)");
	}

	Status result = metaindex_handle_.DecodedFrom(input);
	if (result.ok())
	{
		result = index_handle_.DecodedFrom(input);
	}

	if (result.ok()) {
		const char* end = magic_ptr + 8;

		*input = Slice(end, input->data() + input->size() - end);
	}
	return result;
}


Status ReadBlock(RandomAccessFile* file, const ReadOptions& options, const BlockHandle& handle, BlockContents* result)
{
	result->data = Slice();
	result->cachable = false;
	result->heap_allocated = false;

	size_t n = static_cast<size_t>(handle.size());
	char* buf = new char[n + kBlockTrailerSize];
	Slice contents;
	Status  s = file->Read(handle.offset(), n + kBlockTrailerSize, &contents, buf);

	if (!s.ok()) {
		delete[] buf;
		return s;
	}

	if (contents.size() != n + kBlockTrailerSize) {
		delete[]buf;
		return Status::Corruption("truncated block read");
	}

	const char* data = contents.data();
	if (options.verify_checksums) {
		const  uint32_t crc = crc32c::Unmask(DecodeFixed32(data + n + 1));
		const uint32_t actual = src32c::Value(data, n + 1);

		if (actual != crc) {
			delete[]buf;
			s = Status::Corruption("block checksum mismatch");
		}
	}

	switch (data[n]) {
	case kNoCompression:
		if (data != buf) {
			delete[] buf;
			result->data = Slice(data, n);
			result->heap_allocated = false;
			result->cachable = false;
		}
		else
		{
			result->data = Slice(buf, n);
			result->heap_allocated = true;
			result->cachable = true;
		}
	case kSnappyCompression:
	{

	}
	default:
		delete[] buf;
		return Status::Corruption("bad block type");


	}

	return Status::OK();
}
