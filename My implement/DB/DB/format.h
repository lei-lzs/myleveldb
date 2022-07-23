#include "slice.h"
#include "status.h"
#ifndef _LEVELDB_FORMAT_HH
#define _LEVELDB_FORMAT_HH

class RandomAccessFile;
class ReadOptions;

//记录Block的偏移个大小
class BlockHandle
{
public:
private:
	uint64_t offset_;
	uint64_t size_;
};

class Footer
{
	BlockHandle index_handle_;
	BlockHandle metaindex_block_;

};
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


