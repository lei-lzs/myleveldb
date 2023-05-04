#include "db_iter.h"

#include "iterator.h"
#include "db_format.h"

class DBIter : public Iterator {
public:
	enum Direction { kForward, kReverse};

	DBIter(DBImpl* db, const Comparator* cmp, Iterator* iter, SequenceNumber s, uint32_t seed)
	{

	}

	DBIter(const DBIter&) = delete;
	DBIter& operator = (const DBIter&) = delete;

	~DBIter() override { }

};