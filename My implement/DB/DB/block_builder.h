#ifndef _LEVELDB_BLOCK_BUILDER_HH
#define _LEVELDB_BLOCK_BUILDER_HH

#include <string>
#include <vector>

class Options;
class Slice;

class BlockBuilder
{
public:
	explicit BlockBuilder(const Options* options);
	BlockBuilder(const BlockBuilder&) = delete;
	BlockBuilder& operator = (const BlockBuilder&) = delete;

	void Reset();

	//���һ����Ŀ
	void Add(const Slice& key, const Slice& value);

	Slice Finish();

	//����Block�Ĵ�С
	size_t CurrentSizeEstimate() const;

	bool empty() const;

private:
	const Options* options_;
	std::string buffer_;
	std::vector<uint32_t> restarts_;
	int counter_;
	bool finished_;
	std::string last_key_;

};

#endif
