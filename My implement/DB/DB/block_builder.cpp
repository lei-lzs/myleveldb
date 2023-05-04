#include "block_builder.h"
#include "options.h"
#include "slice.h"
#include "coding.h"
#include <cassert>

BlockBuilder::BlockBuilder(const Options* options)
	:options_(options),
	restarts_(),
	counter_(0),
	finished_(false)
{
	assert(options->block_restart_interval >= 1);
	restarts_.push_back(0); //第一个restart point is offset 0
}

void BlockBuilder::Reset()
{
	buffer_.clear();
	restarts_.clear();
	restarts_.push_back(0);  // First restart point is at offset 0
	counter_ = 0;
	finished_ = false;
	last_key_.clear();
}

void BlockBuilder::Add(const Slice& key, const Slice& value)
{
	//往Block中添加一个KV条目，先都是加到 buffer_ 中

	//KV存储格式 shared_key_len/ non_shared_key_len/ value_len / non_shared key / value
	// key和last_key_ 对比，找到前缀
	//如果此条目是重启点，则restarts_ 加一个记录，counter_计数，看是否是重启点
	uint32_t shared_len, non_shared_len = 0;
	std::string this_key_ = key.ToString();

	size_t min_size = std::min(this_key_.size(), last_key_.size());

	for (size_t i = 0; i < min_size; i++)
	{
		if (last_key_[i] != this_key_[i])
		{
			shared_len = i;
			break;
		}
	}
	non_shared_len = key.size() - shared_len;

	PutVarint32(&buffer_, shared_len);
	PutVarint32(&buffer_, non_shared_len);

	PutVarint32(&buffer_, value.size());

	buffer_.append(key.data() + shared_len, non_shared_len);
	buffer_.append(value.data(), value.size());

	counter_++;

	if (counter_ % options_->block_restart_interval)
	{
		restarts_.push_back(buffer_.size());
	}

	//updata last_key_
	last_key_ = this_key_;
}

//Block Add结束后需要 写Block的 footer
// 这里为什么是PutFixed而不是PutVarint？？？
// 返回Slice(buffer_)
Slice BlockBuilder::Finish()
{
	for (auto i : restarts_)
	{
		PutFixed32(&buffer_, i);
	}

	PutFixed32(&buffer_, restarts_.size());
	
	finished_ = true;

	return Slice(buffer_);
}

size_t BlockBuilder::CurrentSizeEstimate() const
{
	//当前大小故居，后面restart数组是否算多了？？
	return buffer_.size() + restarts_.size()* sizeof(uint32_t) + sizeof(uint32_t);
}

bool BlockBuilder::empty() const
{
	return buffer_.empty();
}
