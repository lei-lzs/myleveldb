#include "filter_block.h"
#include "coding.h"
#include "filter_policy.h"


static const size_t kFilterBaseLg = 11;
static const size_t kFilterBase = 1 << kFilterBaseLg; //2^11 ,2^11B/1024 = 2KB  ,ÿ2KB���ݿ����и�Filter

FilterBlockBuilder::FilterBlockBuilder(const FilterPolicy* policy)
	:policy_(policy)
{

}

//��DataBlock��ʼ��ʱ���Ƿ�Ҫ�½�Filter
void FilterBlockBuilder::StartBlock(uint64_t block_offset)
{
	//���㵱ǰ�������ڵڼ���filter�� block_offset��ʾ DataBlcok���ݵ�ƫ��
	uint64_t filter_index = (block_offset / kFilterBase); 
	while (filter_index > filter_offsets_.size())
	{
		GenerateFilter();
	}
}

void FilterBlockBuilder::AddKey(const Slice& key)
{
	Slice k = key;
	start_.push_back(keys_.size());
	keys_.append(k.data(), k.size());
}

Slice FilterBlockBuilder::Finish()
{
	if (!start_.empty()) {
		GenerateFilter();
	}

	const uint32_t array_offset = result_.size();

	//��¼ÿһ��filter��offset
	for (size_t i = 0; i < filter_offsets_.size(); i++) {
		PutFixed32(&result_, filter_offsets_[i]);
	}

	PutFixed32(&result_, array_offset);

	result_.push_back(kFilterBaseLg);

	return Slice(result_);
}

void FilterBlockBuilder::GenerateFilter()
{
	const size_t num_keys = start_.size();

	if (num_keys == 0) {
		filter_offsets_.push_back(result_.size());
		return;
	}

	start_.push_back(keys_.size());
	tmp_keys_.resize(num_keys);

	for (size_t i = 0; i < num_keys; i++) {
		const char* base = keys_.data() + start_[i];
		size_t length = start_[i + 1] - start_[i];

		tmp_keys_[i] = Slice(base, length);
	}

	filter_offsets_.push_back(result_.size());
	policy_->CreateFilter(&tmp_keys_[0], num_keys, &result_);

	tmp_keys_.clear();
	keys_.clear();
	start_.clear();
}

//Filter Block�� ��ʽ�ܼ�  entry0 +entry1 ..... entryn  + offset0+ offset1
//....offsetn + offset0_offset + base_lg.
FilterBlockReader::FilterBlockReader(const FilterPolicy* policy, const Slice& contents)
	:policy_(policy)
	,data_(nullptr)
	,offset_(nullptr)
	,num_(0)
	,base_lg_(0)
{
	size_t n = contents.size();
	if (n < 5) return;

	//���һ��char תsize_t
	base_lg_ = contents[n - 1];

	//filter offset array��ʼ��λ�ã�����������ֽ�
	uint32_t last_word = DecodeFixed32(contents.data() + n - 5);
	if (last_word > n - 5) return;

	data_ = contents.data();

	//offset array offset
	offset_ = data_ + last_word;

	//filter entry�ĸ�����( offset array �ĳ��� / 4)
	num_ = (n - 5 - last_word) / 4;

}

bool FilterBlockReader::KeyMayMatch(uint32_t block_offset, const Slice& key)
{
	//filter index
	uint64_t index = block_offset >> base_lg_;

	if (index < num_)
	{
		//�ҵ�filter data ����ʼ-����λ�ã� ��������Blockͷ��λ���𣿣���
		uint32_t start = DecodeFixed32(offset_ + index * sizeof(uint32_t));
		uint32_t limit = DecodeFixed32(offset_ +index* sizeof(uint32_t)+4);

		if (start <= limit && limit <= (offset_ - data_))
		{
			Slice filter = Slice(data_ + start, limit - start);
			return policy_->KeyMayMatch(key, filter);
		}
		else if (start = limit)
		{
			return false;
		}
	}
	return true;
}
