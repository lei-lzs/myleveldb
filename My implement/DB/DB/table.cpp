#include "table.h"
#include "format.h"
#include "block.h"
#include "filter_block.h"
#include "options.h"

struct Table::Rep {
	~Rep() {

	}

	Options options;
	Status status;
	RandomAccessFile* file;
	uint64_t cache_id;
	FilterBlockReader* filter;
	const char* filter_data;

	BlockHandle metaindex_handle;
	Block* index_block;
};



Status Table::Open(const Options& options, RandomAccessFile* file, uint64_t size, Table** table)
{
	*table = nullptr;

	//file size is small than the footer length
	if (size < Footer::kEncodedLength) {
		return Status::Corruption("file is too short to be an sstable");
	}

	//Footer 固定长度
	char footer_sapce[Footer::kEncodedLength];
	Slice footer_input;
	Status s = file->Read(size - Footer::kEncodedLength, Footer::kEncodedLength, &footer_input, footer_sapce);

	if (!s.ok()) return s;

	Footer footer;
	s = footer.DecodedFrom(&footer_input);
	if (!s.ok()) return s;

	// raed the index block 
	BlockContents index_block_contents;
	ReadOptions opt;
	if (options.paranoid_checks) {
		opt.verify_checksums = true;
	}

	//读取index_block的内容
	s = ReadBlock(file, opt, footer.index_handle(), &index_block_contents);

	if (s.ok()) {
		Block* index_block = new Block(index_block_contents);
		Rep* rep = new Table::Rep;

		rep->options = options;
		rep->file = file;
		rep->metaindex_handle = footer.metaindex_handle();
		rep->index_block = index_block;
		rep->cache_id = (options.block_cache ? options.block_cache->NewId() : 0);
		rep->filter_data = nullptr;
		rep->filter = nullptr;
		*table = new Table(rep);
		(*table)->ReadMeta(footer);
	}
	return s;
}

Table::~Table()
{
}

Iterator* Table::NewIterator(const ReadOptions&) const
{
	return nullptr;
}

uint64_t Table::ApproximateOffsetOf(const Slice& key) const
{
	return uint64_t();
}


Iterator* Table::BlockReader(void*, const ReadOptions&, const Slice&)
{
	return nullptr;
}

Status Table::InternalGet(const ReadOptions&, const Slice& key, void* arg, void(*handle_result)(void* arg, const Slice& k, const Slice& v))
{
	return Status();
}

void Table::ReadMeta(const Footer& footer)
{
	ReadFilter();
}

void Table::ReadFilter(const Slice& filter_handle_value)
{
}
