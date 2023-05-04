#ifndef _LEVELDB_DBIMPL_HH
#define _LEVELDB_DBIMPL_HH

#include "db.h"
#include "port_stdcxx.h"

#include <mutex>
#include <deque>

class VersionSet;
class MemTable;

class DBImpl: public DB
{
public:
	DBImpl(const Options& options, const std::string& dbname);
	virtual ~DBImpl();

	//interface implementation
	virtual Status Put(const WriteOptions& options,
		const Slice& key,
		const Slice& value);

	virtual Status Delete(const WriteOptions& options, const Slice& key);

	virtual Status Write(const WriteOptions& options, WriteBatch* updates);

	virtual Status Get(const ReadOptions& options,
		const Slice& key, std::string* value);

	virtual Iterator* NewIterator(const ReadOptions& options);

private:

	Status MakeRoomForWrite(bool force /* compact even if there is room? */);

	Status bg_error_;

	Env* const env_;
	const Options options_;  // options_.comparator == &internal_comparator_
		
	struct Writer;

	port::Mutex mutex_;

	MemTable* mem_;
	MemTable* imm_ GUARDED_BY(mutex_);  // Memtable being compacted

	// Queue of writers.
	std::deque<Writer*> writers_;

	VersionSet* const versions_;
};


#endif
