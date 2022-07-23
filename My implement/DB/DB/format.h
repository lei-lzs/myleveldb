#include "slice.h"
#include "status.h"
#ifndef _LEVELDB_FORMAT_HH
#define _LEVELDB_FORMAT_HH

class RandomAccessFile;
class ReadOptions;

//��¼Block��ƫ�Ƹ���С
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
//BlockContents���ڱ���Block�Ľ��
Status ReadBlock(RandomAccessFile* file, const ReadOptions& options,
	const BlockHandle& handle, BlockContents* result);

#endif


