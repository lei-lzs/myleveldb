#ifndef _LEVELDB_HH
#define _LEVELDB_HH

class DBImpl;

Iterator* NewDBIterator(DbImpl* db, const Comparator* user_key_comparator,
	Iterator* internal_iter, SequenceNumber sequence, uint32_t seed);


#endif