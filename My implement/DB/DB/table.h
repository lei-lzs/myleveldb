#ifndef _LEVELDB_TABLE_HH
#define _LEVELDB_TABLE_HH

#include "status.h"

class Options;
class ReadOptions;
class RandomAccessFile;

class Iterator;
class Footer;

//sorted string table , �洢<string,string> 
//table �ǳ־õĲ��ɱ�ģ��̰߳�ȫ�ģ� ��α�֤�̰߳�ȫ
//��Ҫ֪��table����ϸ�洢��ʽ
//��Ҫ�ӿڣ�
/*
* Open(), ��һ��Table
* NewIterator(), Table�ĵ�����
* ��ȡTable���ݵ�һЩ�ӿڣ���  BlockReader�� ReadMeta�� ReadFilter
*/


//Table���ڶ�ȡ����Table
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
