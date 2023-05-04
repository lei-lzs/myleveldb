#include "slice.h"
#include "status.h"
#ifndef _LEVELDB_FORMAT_HH
#define _LEVELDB_FORMAT_HH

class RandomAccessFile;
class ReadOptions;

//记录Block的偏移和大小, 文件位置的指针
class BlockHandle
{
public:

	//uint64_t 最大编码长度是10， 7位 一编码
	enum {
		kMaxEncodedLength = 10 + 10
	};

	BlockHandle():offset_(uint64_t(-1)), size_(uint64_t(-1)){}
	uint64_t offset() const { return offset_; }
	void set_offset(uint64_t offset) { offset_ = offset; }

	uint64_t size() const { return size_; }
	uint64_t set_size(uint64_t size) { size_ = size; }

	void EncodeTo(std::string* dst) const;
	Status DecodedFrom(Slice* input);

private:
	uint64_t offset_;
	uint64_t size_;
};

//文件尾部, 记录了index_block 和 metaindex_block的位置
class Footer
{
public:
	enum { kEncodedLength = 2*BlockHandle::kMaxEncodedLength + 8};

	Footer() = default;

	const BlockHandle& metaindex_handle() const { return metaindex_handle_; }
	void set_metaindex_handle(const BlockHandle& h) { metaindex_handle_ = h; }

	const BlockHandle& index_handle() const { return index_handle_; }
	void set_index_handle(const BlockHandle & h) { index_handle_ = h; }

	void EncodeTo(std::string* dst) const;
	Status DecodedFrom(Slice* input);
private:
	BlockHandle index_handle_;
	BlockHandle metaindex_handle_;
};

static const uint64_t kTableMagicNumber = 0xdb4775248b80fb57ull;

// 1-byte type + 32-bit crc
static const size_t kBlockTrailerSize = 5;

struct BlockContents
{
	Slice data;
	bool cachable;
	bool heap_allocated;
};


//Read the Block identified by BlockHandle from file.
//BlockContents用于保存Block的结果
Status ReadBlock(RandomAccessFile* file, const ReadOptions& options,
	const BlockHandle& handle, BlockContents* result);

#endif


