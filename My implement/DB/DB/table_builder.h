#ifndef _LEVELDB_TABLE_BUILDER_HH
#define _LEVELDB_TABLE_BUILDER_HH

#include "status.h"

class Options;
class WritableFile;
class Slice;
class BlockBuilder;
class BlockHandle;

//用于构建Tabel，和Table类的过程相反
//如果MemTable(SkipList)中的数据达到一定大小，就会构建Level0 Table。
class TableBuilder
{
public:
	TableBuilder(const Options& options, WritableFile* file);

	TableBuilder(const TableBuilder&) = delete;
	TableBuilder& operator= (const TableBuilder&) = delete;

	~TableBuilder();

	Status ChangeOptions(const Options& option);

	void Add(const Slice& key, const Slice& value);

	void Flush();

	Status status() const;

	Status Finish();

	void Abandon();

	uint64_t NumEntries() const;

	uint64_t FileSize() const;

private:
	bool ok() const { return status().ok(); }

	void WriteBlock(BlockBuilder* block, BlockHandle* handle);

	void WriteRawBlock(const Slice& data, CompressionType, BlockHandle* handle);

	struct Rep;
	Rep* rep_;
};

#endif