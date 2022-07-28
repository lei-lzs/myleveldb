#include <cstdint>
#ifndef _LEVELDB_DB_FORMAT_HH
#define _LEVELDB_DB_FORMAT_HH


namespace config {
	static const int kNumLevels = 7;

	// Level-0 compaction is started when we hit this many files.
	static const int kL0_CompactionTrigger = 4;

	// Soft limit on number of level-0 files.  We slow down writes at this point.
	static const int kL0_SlowdownWritesTrigger = 8;

	// Maximum number of level-0 files.  We stop writes at this point.
	static const int kL0_StopWritesTrigger = 12;

	// Maximum level to which a new compacted memtable is pushed if it
	// does not create overlap.  We try to push to level 2 to avoid the
	// relatively expensive level 0=>1 compactions and to avoid some
	// expensive manifest file operations.  We do not push all the way to
	// the largest level since that can generate a lot of wasted disk
	// space if the same key space is being repeatedly overwritten.
	static const int kMaxMemCompactLevel = 2;

	// Approximate gap in bytes between samples of data read during iteration.
	static const int kReadBytesPeriod = 1048576;

}  // namespace config

enum ValueType { kTypeDeletion = 0x0, kTypeValue = 0x1 };

typedef uint64_t SequenceNumber;

//��InternaKey�ķ�װ
//UserKey+ SequenceNum + type 
class InternalKey
{

};

//
//Variant length + InternalKey
class LookupKey
{

};

#endif