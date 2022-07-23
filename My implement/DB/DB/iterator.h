#include "status.h"
#ifndef _LEVELDB_ITERATOR_HH
#define _LEVELDB_ITERATOR_HH


class Slice;

class Iterator
{
public:
	Iterator(); //ÎªÊ²Ã´ÊÇpublic?

	Iterator(const Iterator&) = delete;
	Iterator& operator=(const Iterator&) = delete;

	virtual ~Iterator();

	virtual bool Valid() const = 0;
	virtual void SeekToFirst() = 0;
	virtual void SeekTolast() = 0;
	virtual void Seek(const Slice& target) = 0;

	virtual void Next() = 0;
	virtual void Prev() = 0;
	virtual Slice key() = 0;
	virtual Slice value() = 0;
	virtual Status status() = 0;

	using CleanupFunction = void (*)(void* arg1, void* arg2);
	void RegisterCleanup(CleanupFunction function, void* arg1, void* arg2);

};

#endif