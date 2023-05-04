#include "memtable.h"
#include "Slice.h"
#include "coding.h"
#include "iterator.h"

MemTable::MemTable(const InternalKeyComparator& comparator)
	:comparator_(comparator), refs_(0)
{

}

size_t MemTable::ApproximateMemoryUsage()
{
	//return table_.size();
	return arena_.MemoryUsage();
}

//ֱ�ӶԵ�����е���
class MemTableIterator : public Iterator
{
public:
	explicit MemTableIterator(MemTable::Table* table_)
		:iter_(table_){}

	virtual bool Valid() const {}
	virtual void SeekToFirst() {}
	virtual void SeekTolast() {}
	virtual void Seek(const Slice& target) {}

	virtual void Next() {}
	virtual void Prev() {}
	virtual Slice key() {}
	virtual Slice value() {}
	virtual Status status() {}

private:
	MemTable::Table* iter_;
};


Iterator* MemTable::NewIterator()
{
	return new MemTableIterator(&table_);
}

void MemTable::Add(SequenceNumber seq, ValueType type, const Slice& key, const Slice& value)
{
	//��K-V �洢�� Table
	//format: key_size+ key + tag[sequence+ type] + value_size +value
	size_t key_size = key.size();
	size_t value_size = value.size();
	size_t internal_key_size = key_size+8; //user_key + sequence +type
	const size_t encoded_len = VarintLength(internal_key_size) + internal_key_size +
		VarintLength(value_size) + value_size;

	char* buf = arena_.Allocate(encoded_len);
	char* p = EncodeVarint32(buf, internal_key_size);
	std::memcpy(p,key.data(),key_size);

	p += key_size;
	EncodeFixed64(p, (seq << 8) | type);
	p = EncodeVarint32(p,value_size);
	std::memcpy(p, value.data(), value_size);
	assert(p + value_size == buf + encoded_len);

	//table_.insert(buf);

	
}

bool MemTable::Get(const LookupKey& key, std::string* value, Status* s)
{
	//��LookupKey �õ�key
	//��SkipList�� ��ȡkv��¼
	//������value
	Slice memkey = key.memtable_key();
	return false;
}
