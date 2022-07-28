
#ifndef _STATUS_HH
#define _STATUS_HH

#include "slice.h"

class Status
{
public:
	Status() :state_(nullptr) {};
	~Status() { delete []state_; };

	Status(const Status& s);
	Status& operator =(const Status& s);

	Status(Status&& s);
	Status& operator =(Status&& s);

	static Status OK() {return Status(); }

	static Status NotFound(const Slice& msg, const Slice& msg2 = Slice()) {
		return Status(kNotFound, msg, msg2); 
	}

	static Status Corruption(const Slice& msg, const Slice& msg2 = Slice()) {
		return Status(kCorruption, msg, msg2);
	}

	static Status NotSupported(const Slice& msg, const Slice& msg2 = Slice()) {
		return Status(kNotSupported, msg, msg2);
	}

	static Status InvalidArgument(const Slice& msg, const Slice& msg2 = Slice()) {
		return Status(kInvalidArgument, msg, msg2);
	}

	static Status IOError(const Slice& msg, const Slice& msg2 = Slice()) {
		return Status(kIOError, msg, msg2);
	}

	bool ok() { return state_ == nullptr; }
	bool IsNotFound() { return code() == kNotFound; }
	bool IsCorruption() { return code() == kCorruption; }
	bool IsNotSupported() { return code() == kNotSupported; }
	bool IsInvalidArgument() { return code() == kInvalidArgument; }
	bool IsIOError() { return code() == kIOError; }

	std::string ToString() const;

private:
	enum Code {
		kOk = 0,
		kNotFound,
		kCorruption = 2,
		kNotSupported = 3,
		kInvalidArgument = 4,
		kIOError = 5
	};

	Code code() const { return static_cast<Code>(state_[4]); }

	Status(Code code, const Slice& msg, const Slice& msg2);

	//用于实现拷贝构造
	const char* CopyStatus(const char* s);

	// OK status has a null state_.  Otherwise, state_ is a new[] array
	// of the following form:
	//    state_[0..3] == length of message
	//    state_[4]    == code
	//    state_[5..]  == message

	const char* state_;
	//指针可以变，指向不同的字符串
	//但是不能通过指针改变字符串
};

#endif
