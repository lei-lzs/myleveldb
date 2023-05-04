#ifndef _LEVELDB_MEMTABLE_HH
#define _LEVELDB_MEMTABLE_HH

#include "db_format.h"
#include <string>
#include <cassert>
#include "SkipList.h"
#include "arena.h"

class InternalKeyComparator;
class Slice;
class Iterator;
class Status;

//内存中的Table， 对SkipList的封装
/*
* 接口：引用计数
* Add() 添加记录
* Get() 获取记录
* Iterator 迭代器
*/

class MemTable {
public:
	explicit MemTable(const InternalKeyComparator& comparator);

	MemTable(const MemTable&) = delete;
	MemTable& operator =(const MemTable&) = delete;

	void Ref() { ++refs_; }
	void Unref() {
		--refs_;
		assert(refs_ >= 0);
		if (refs_ <= 0) {
			delete this;
		}
	}

	size_t ApproximateMemoryUsage();

	Iterator* NewIterator();

	void Add(SequenceNumber seq, ValueType type, const Slice& key, const Slice& value);

	bool Get(const LookupKey& key, std::string* value, Status* s);

private:
	friend class MemTableIterator;
	friend class MemTableBcakwardIterator;

	struct KeyComparator {
		const InternalKeyComparator comparator;
		explicit KeyComparator(const InternalKeyComparator& c) : comparator(c){}

		int operator() (const char* a, const char* b) const;
	};

	typedef SkipList Table;

	~MemTable();

	KeyComparator comparator_;
	int refs_;
	Arena arena_;
	Table table_;

};

#endif
