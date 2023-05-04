#include "table_builder.h"
#include "options.h"
#include "block_builder.h"
#include "filter_block.h"
#include "format.h"
#include "crc32c.h"
#include "comparator.h"
#include "coding.h"



//构建Table所需的 内部数据结构，包括文件，选项，不同的BlockBuilder
struct TableBuilder::Rep {
	Rep(const Options& opt, WritableFile* f)
		:options(opt)
		,index_block_option(opt)
		,file(f)
		,offset(0)
		,data_block(&opt)
		,index_block(&opt)
		,num_entries(0)
		,filter_block(opt.filter_policy == nullptr ? nullptr: new FilterBlockBuilder(opt.filter_policy))
		,pending_index_entry(false)
	{
		//index block 的restart间隔是1
		index_block_option.block_restart_interval = 1;
	}
	Options options;
	Options index_block_option;
	WritableFile* file;
	uint64_t offset;
	Status status;
	BlockBuilder data_block;
	BlockBuilder index_block;
	std::string last_key;
	int64_t num_entries;
	bool closed;
	FilterBlockBuilder* filter_block;
	bool pending_index_entry;
	BlockHandle pending_handle;
	std::string compressed_output;
};

TableBuilder::TableBuilder(const Options& options, WritableFile* file)
	:rep_(new Rep(options,file))
{
	if (rep_->filter_block != nullptr) {
		rep_->filter_block->StartBlock(0);  //这里在干嘛
	}
}

TableBuilder::~TableBuilder()
{
	assert(rep_->closed);
	delete rep_->filter_block;
	delete rep_;
}

Status TableBuilder::ChangeOptions(const Options& option)
{
	return Status();
}

void TableBuilder::Add(const Slice& key, const Slice& value)
{
	Rep* r = rep_;
	assert(r->closed);
	if (!ok()) return;
	if (r->num_entries > 0) {
		assert(r->options.comparator->Compare(key, Slice(r->last_key)) > 0);
	}

	if (r->pending_index_entry) { //DataBlock为空时 才为ture, 开启新的Block的时候需要记录 index block
		//这里的key 是大于等于上一个Block最后一个ley 小于 下一个Block第一个key的key
		//为什么这样？？  满足左开右闭的二分查找？
		r->options.comparator->FindShorestSeparator(&r->last_key, key);
		std::string handle_encoding;
		r->pending_handle.EncodeTo(&handle_encoding); //上一个block的 offset

		r->index_block.Add(r->last_key, Slice(handle_encoding));
		r->pending_index_entry = false;
	}

	if (r->filter_block != nullptr) {
		r->filter_block->AddKey(key);
	}

	r->last_key.assign(key.data(), key.size());
	r->num_entries++;
	r->data_block.Add(key, value);  //restart point 在接口里面完成

	const size_t estimated_block_size = r->data_block.CurrentSizeEstimate();

	if (estimated_block_size > r->options.block_size) {
		Flush();
	}
}

//如果写入的数据超过block_size ,就需要Flush，生成一个DataBlock，写入文件
void TableBuilder::Flush()
{
	Rep* r = rep_;
	assert(!r ->closed);
	if (!ok()) return;
	if (r->data_block.empty()) return;
	assert(!r->pending_index_entry);

	WriteBlock(&r->data_block, &r->pending_handle);
	if (ok()) {
		r->pending_index_entry = true;
		r->status = r->file->Flush();
	}

	if (r->filter_block != nullptr)
	{
		r->filter_block->StartBlock(r->offset);
	}
}

Status TableBuilder::status() const
{
	return Status();
}

Status TableBuilder::Finish()
{
	return Status();
}

void TableBuilder::Abandon()
{
}

uint64_t TableBuilder::NumEntries() const
{
	return uint64_t();
}

uint64_t TableBuilder::FileSize() const
{
	return uint64_t();
}

void TableBuilder::WriteBlock(BlockBuilder* block, BlockHandle* handle)
{
// File format contains a sequence of blocks where each block has:
 //    block_data: uint8[n]
 //    type: uint8
 //    crc: uint32
	assert(ok());
	Rep* r = rep_;

	//获取BlockBuilder的缓存数据
	Slice raw = block->Finish();

	Slice block_contents;
	CompressionType type = r->options.compression;

	switch (type)
	{
	case kNoCompression:
	{
		block_contents = raw;
		break;
	}

	case kSnappyCompression:
	{
		std::string* compressed = &r->compressed_output;
		if (port::Snappy_Compress(raw.data(), raw.size(), compressed) &&
			compressed->size() < raw.size() - (raw.size() / 8u)) {
			block_contents = *compressed;
		}
		else {
			block_contents = raw;
			type = kNoCompression;
		}
		break;
	}
	}

	WriteRawBlock(block_contents, type, handle);
	r->compressed_output.clear();
	block->Reset();
}

void TableBuilder::WriteRawBlock(const Slice& block_contents, CompressionType type, BlockHandle* handle)
{
	Rep* r = rep_;
	handle->set_offset(r->offset);
	handle->set_size(block_contents.size());

	r->status = r->file->Append(block_contents);
	if (r->status.ok()) {
		char trailer[kBlockTrailerSize];
		trailer[0] = type;
		uint32_t crc = crc32c::Value(block_contents.data(), block_contents.size());
		crc = crc32c::Extend(crc, trailer, 1);  // Extend crc to cover block type
		EncodeFixed32(trailer + 1, crc32c::Mask(crc));

		r->status = r->file->Append(Slice(trailer, kBlockTrailerSize));
		if (r->status.ok()) {
			r->offset += block_contents.size() + kBlockTrailerSize;
		}

	}
}
