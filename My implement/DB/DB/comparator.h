#ifndef _LEVELDB_COMPARATOR_HH
#define _LEVELDB_COMPARATOR_HH

class Slice;

//A comparator provider a total order across slices that are used as the keys in an sstable or a database.
//A comparator implementation must be thread safe since leveldb may be invoke concurrently.

class Comparator
{
public:
	virtual ~Comparator();

	virtual int Compare(const Slice& a, const Slice& b) const = 0;

	virtual const char* Name() const = 0;

};

#endif