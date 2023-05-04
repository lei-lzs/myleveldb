#include <cstdint>
#ifndef _LEVELDB_BLOCK_HH
#define _LEVELDB_BLOCK_HH

class BlockContents;
class Iterator;
class Comparator;

//Block是SSTable中块，包括库DataBlock，IndexBlock等
//Block 记录了？
//Block 通过Iterator 访问

//用于存储解析Block
class Block
{
public:
	explicit Block(const BlockContents& contents);

	Block(const Block&) = delete;
	Block& operator=(const Block&) = delete;

	~Block();

	size_t size() const { return size_; }

	Iterator* NewIterator(const Comparator* comparator);

private:

	class Iter;

	uint32_t NumRestarts() const;

	const char* data_;
	size_t size_;
	//restart数组的 offset
	uint32_t restart_offset_;
	bool owned_;


};

#endif

