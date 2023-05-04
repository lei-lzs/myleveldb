#include "db_format.h"
#include "coding.h"
#include "logging.h"
#include <sstream>

static uint64_t PackSequenceAndType(uint64_t seq, ValueType t) {
	assert(seq <= kMaxSequenceNumber);
	assert(t <= kValueTypeForSeek);
	return ((seq << 8) | t);
}

LookupKey::LookupKey(const Slice& user_key, SequenceNumber sequence)
{
	size_t usize = user_key.size();
	size_t needed = usize + 13; 
	char* dst;

	if (needed <= sizeof(space_)) {
		dst = space_;
	}
	else {
		dst = new char[needed];
	}

	start_ = dst;
	dst = EncodeVarint32(dst, usize + 8);

	kstart_ = dst;

	std::memcpy(dst, user_key.data(), usize);
	dst += usize;

	EncodeFixed64(dst, PackSequenceAndType(sequence, kValueTypeForSeek));
	dst += 8;
	end_ = dst;
}

std::string ParsedInternalKey::DebugString() const
{
	std::ostringstream ss;

	ss << '\'' << EscapeString(user_key.ToString()) << "'@" << sequence << ":" << static_cast<int>(type);

	return ss.str();
}

size_t InternalKeyEncodingLength(const ParsedInternalKey& key)
{
	return key.user_key.size() + 8;
}

void AppendInternalKey(std::string* result, const ParsedInternalKey& key)
{
	result->append(key.user_key.data(), key.user_key.size());
	PutFixed64(result, PackSequenceAndType(key.sequence, key.type));
}

bool ParseInternalKey(const Slice& internal_key, ParsedInternalKey* result)
{ 
	// user_key + sequence + type
	//从internal_key中解析出各个字段
	size_t size = internal_key.size();
	assert(size >= 8);
	const char* dst = internal_key.data();
	result->user_key = Slice(dst, size - 8);
	uint64_t tail = DecodeFixed64(dst + size - 8);
	result->type = (ValueType)static_cast<uint8_t>(tail);
	result->sequence = tail >>8;	
	return result->type < kTypeValue;
}

std::string InternalKey::DebugString() const
{
	ParsedInternalKey parsed;

	if (ParseInternalKey(rep_, &parsed)) {
		return parsed.DebugString();
	}

	std::ostringstream ss;

	ss << "(bad)" << EscapeString(rep_);

	return ss.str();
}
