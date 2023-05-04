#ifndef _LEVELDB_DB_FORMAT_HH
#define _LEVELDB_DB_FORMAT_HH

#include <cstdint>
#include "comparator.h"
#include "slice.h"

namespace config {
	static const int kNumLevels = 7;

	// Level-0 compaction is started when we hit this many files.
	static const int kL0_CompactionTrigger = 4;

	// Soft limit on number of level-0 files.  We slow down writes at this point.
	static const int kL0_SlowdownWritesTrigger = 8;

	// Maximum number of level-0 files.  We stop writes at this point.
	static const int kL0_StopWritesTrigger = 12;

	// Maximum level to which a new compacted memtable is pushed if it
	// does not create overlap.  We try to push to level 2 to avoid the
	// relatively expensive level 0=>1 compactions and to avoid some
	// expensive manifest file operations.  We do not push all the way to
	// the largest level since that can generate a lot of wasted disk
	// space if the same key space is being repeatedly overwritten.
	static const int kMaxMemCompactLevel = 2;

	// Approximate gap in bytes between samples of data read during iteration.
	static const int kReadBytesPeriod = 1048576;

}  // namespace config

enum ValueType { kTypeDeletion = 0x0, kTypeValue = 0x1 };

static const ValueType kValueTypeForSeek = kTypeValue;  //这个是干什么的？

typedef uint64_t SequenceNumber;

static const SequenceNumber kMaxSequenceNumber = ((0x1ull << 56) - 1);

//解析过的InternalKey，包括三个字段
struct ParsedInternalKey {
	Slice user_key;
	SequenceNumber sequence;
	ValueType type;

	ParsedInternalKey() {}

	ParsedInternalKey(const Slice& u, const SequenceNumber& seq, ValueType t)
		: user_key(u),sequence(seq), type(t){
	}

	std::string DebugString() const;
};

size_t InternalKeyEncodingLength(const ParsedInternalKey& key);

void AppendInternalKey(std::string* result, const ParsedInternalKey& key);

bool ParseInternalKey(const Slice& internal_key, ParsedInternalKey* result);

Slice ExtractUserKey(const Slice& internal_key) {
	assert(internal_key.size() >= 8);
	return Slice(internal_key.data(), internal_key.size() - 8);
}


//对InternaKey的封装
//UserKey+ SequenceNum + type 
class InternalKey
{
private:
	std::string rep_;
public:
	InternalKey() {}

	InternalKey(const Slice& user_key, SequenceNumber s, ValueType t) {
		AppendInternalKey(&rep_, ParsedInternalKey(user_key, s, t));
	}

	bool DecodeFrom(const Slice& s) {
		rep_.assign(s.data(), s.size());
		return !rep_.empty();
	}

	Slice Encode() const {
		return rep_;
	}

	Slice user_key() const {
		return ExtractUserKey(rep_);
	}

	void SetFrom(const ParsedInternalKey& p) {
		rep_.clear();
		AppendInternalKey(&rep_, p);
	}

	void Clear() { rep_.clear(); }

	std::string DebugString() const;
};

//
//Variant length + InternalKey
class LookupKey
{
public:
	LookupKey(const Slice& user_key, SequenceNumber sequence);

	LookupKey(const LookupKey&) = delete;
	LookupKey& operator = (const LookupKey&) = delete;

	~LookupKey() { if (start_ != space_) delete[]start_; }

	Slice memtable_key() const { return Slice(start_, end_-start_); }
	Slice internal_key() const { return Slice(kstart_, end_ - kstart_); }
	Slice user_key() const { return Slice(kstart_, end_ - kstart_ - 8); }

private:
	//kLength  varint32               start_
	//userKey  char[kLength]		  kstart_
	//tag      uint64				  end_	

	const char* start_;
	const char* kstart_;
	const char* end_;
	char space_[200];




};

class InternalKeyComparator : public Comparator {
private:
	const Comparator* user_comparator_;
public:
	explicit InternalKeyComparator(const Comparator* c): user_comparator_(c){}
	const char* Name() const override;
	int Compare(const Slice& a, const Slice& b) const override;
	void FindShortestSepatator(std::string* start, const Slice& limit) const;

};

#endif