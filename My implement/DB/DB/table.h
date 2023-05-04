#ifndef _LEVELDB_TABLE_HH
#define _LEVELDB_TABLE_HH

#include "status.h"

class Options;
class ReadOptions;
class RandomAccessFile;

class Iterator;
class Footer;

//sorted string table , 存储<string,string> 
//table 是持久的不可变的，线程安全的， 如何保证线程安全
//需要知道table的详细存储格式
//主要接口：
/*
* Open(), 打开一个Table
* NewIterator(), Table的迭代器
* 读取Table内容的一些接口，如  BlockReader， ReadMeta， ReadFilter
*/


//Table用于读取解析Table
class Table
{
	static Status Open(const Options& options, RandomAccessFile* file,
		uint64_t sile_size, Table** table);

	Table(const Table&) = delete;
	Table& operator = (const Table&) = delete;

	~Table();

	Iterator* NewIterator(const ReadOptions&) const;

	uint64_t ApproximateOffsetOf(const Slice& key) const;

private:
	friend class TableCache;
	struct Rep;

	static Iterator* BlockReader(void*, const ReadOptions&, const Slice&);

	explicit Table(Rep* rep): rep_(rep) {}

	Status InternalGet(const ReadOptions&, const Slice& key, void* arg,
		void(*handle_result)(void* arg, const Slice& k, const Slice& v));

	void ReadMeta(const Footer& footer);
	void ReadFilter(const Slice& filter_handle_value);

	Rep* const rep_;

};

#endif
