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
	restarts_.push_back(0); //��һ��restart point is offset 0
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
	//��Block�����һ��KV��Ŀ���ȶ��Ǽӵ� buffer_ ��

	//KV�洢��ʽ shared_key_len/ non_shared_key_len/ value_len / non_shared key / value
	// key��last_key_ �Աȣ��ҵ�ǰ׺
	//�������Ŀ�������㣬��restarts_ ��һ����¼��counter_���������Ƿ���������
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

//Block Add��������Ҫ дBlock�� footer
// ����Ϊʲô��PutFixed������PutVarint������
// ����Slice(buffer_)
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
	//��ǰ��С�ʾӣ�����restart�����Ƿ�����ˣ���
	return buffer_.size() + restarts_.size()* sizeof(uint32_t) + sizeof(uint32_t);
}

bool BlockBuilder::empty() const
{
	return buffer_.empty();
}
