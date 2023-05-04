#ifndef _LEVELDB_VERSION_SET_HH
#define _LEVELDB_VERSION_SET_HH

#include <vector>
#include "db_format.h"
#include "log_writer.h"

class FileMetaData;
class TableCache;
class WritableFile;
class ReadOptions;
class Iterator;
class Options;
class Env;

class Version {
public:

	struct GetStats {
		FileMetaData* seek_file;
		int seek_file_level;
	};

	//将需要合并的文件的Iterator加入到iters
	void AddIterators(const ReadOptions&, std::vector<Iterator*>* iters);

	Status Get(const ReadOptions&, const LookupKey& key, std::string* val, GetStats* status);

	bool UpdateStats(const GetStats& stats);

	bool RecordReadSample(Slice key);

	void Ref();
	void Unref();

	void GetOverlappingInputs(int level, const InternalKey* begin, const InternalKey* end,
		std::vector<FileMetaData*>* inputs);

	//判断某个level的文件是否重叠
	bool OverlapInLevel(int level,
		const Slice* smallest_user_key,
		const Slice* largest_user_key);

	int PickLevelForMemtableOutput(const Slice& smallest_user_key,
		const Slice& largest_user_key);

	int NumFiles(int level) const { return files_[level].size(); }

	std::string DebugString() const;
private:

	Iterator* NewConcatenatingIterator(const ReadOptions&, int level) const;

	void ForEachOverlapping(Slice user_key, Slice internal_key, void* arg,
		bool (*func)(void*, int, FileMetaData*));

	VersionSet* vset_;
	Version* next_;
	Version* prev_;
	int refs_;

	//vector 的数组  每个vector代表 一个level上的所有file
	std::vector<FileMetaData*> files_[config::kNumLevels];

	//基于一些规则算出来的下一个待compact的文件和所在level
	FileMetaData* file_to_compact_;
	int file_to_compact_level_;

	double compaction_score_;
	int compaction_level_;
};

//VersionSet是由Version节点组成的双向链表
class VersionSet {
public:
	VersionSet(const std::string& dbname,
		const Options* options,
		TableCache* table_cache,
		const InternalKeyComparator*);
	~VersionSet();

	// Apply *edit to the current version to form a new descriptor that
	// is both saved to persistent state and installed as the new
	// current version.  Will release *mu while actually writing to the file.
	// REQUIRES: *mu is held on entry.
	// REQUIRES: no other thread concurrently calls LogAndApply()
	Status LogAndApply(VersionEdit* edit, port::Mutex* mu)
		EXCLUSIVE_LOCKS_REQUIRED(mu);

	// Recover the last saved descriptor from persistent storage.
	Status Recover(bool* save_manifest);

	// Return the current version.
	Version* current() const { return current_; }

	// Return the current manifest file number
	uint64_t ManifestFileNumber() const { return manifest_file_number_; }

	// Allocate and return a new file number
	uint64_t NewFileNumber() { return next_file_number_++; }

	// Arrange to reuse "file_number" unless a newer file number has
	// already been allocated.
	// REQUIRES: "file_number" was returned by a call to NewFileNumber().
	void ReuseFileNumber(uint64_t file_number) {
		if (next_file_number_ == file_number + 1) {
			next_file_number_ = file_number;
		}
	}

	// Return the number of Table files at the specified level.
	int NumLevelFiles(int level) const;

	// Return the combined file size of all files at the specified level.
	int64_t NumLevelBytes(int level) const;

	// Return the last sequence number.
	uint64_t LastSequence() const { return last_sequence_; }

	// Set the last sequence number to s.
	void SetLastSequence(uint64_t s) {
		assert(s >= last_sequence_);
		last_sequence_ = s;
	}

	// Mark the specified file number as used.
	void MarkFileNumberUsed(uint64_t number);

	// Return the current log file number.
	uint64_t LogNumber() const { return log_number_; }

	// Return the log file number for the log file that is currently
	// being compacted, or zero if there is no such log file.
	uint64_t PrevLogNumber() const { return prev_log_number_; }

	// Pick level and inputs for a new compaction.
	// Returns nullptr if there is no compaction to be done.
	// Otherwise returns a pointer to a heap-allocated object that
	// describes the compaction.  Caller should delete the result.
	Compaction* PickCompaction();

	// Return a compaction object for compacting the range [begin,end] in
	// the specified level.  Returns nullptr if there is nothing in that
	// level that overlaps the specified range.  Caller should delete
	// the result.
	Compaction* CompactRange(
		int level,
		const InternalKey* begin,
		const InternalKey* end);

	// Return the maximum overlapping data (in bytes) at next level for any
	// file at a level >= 1.
	int64_t MaxNextLevelOverlappingBytes();

	// Create an iterator that reads over the compaction inputs for "*c".
	// The caller should delete the iterator when no longer needed.
	Iterator* MakeInputIterator(Compaction* c);

	// Returns true iff some level needs a compaction.
	bool NeedsCompaction() const {
		Version* v = current_;
		return (v->compaction_score_ >= 1) || (v->file_to_compact_ != nullptr);
	}

	// Add all files listed in any live version to *live.
	// May also mutate some internal state.
	void AddLiveFiles(std::set<uint64_t>* live);

	// Return the approximate offset in the database of the data for
	// "key" as of version "v".
	uint64_t ApproximateOffsetOf(Version* v, const InternalKey& key);

	// Return a human-readable short (single-line) summary of the number
	// of files per level.  Uses *scratch as backing store.
	struct LevelSummaryStorage {
		char buffer[100];
	};
	const char* LevelSummary(LevelSummaryStorage* scratch) const;

private:
	class Builder;

	friend class Compaction;
	friend class Version;

	bool ReuseManifest(const std::string& dscname, const std::string& dscbase);

	void Finalize(Version* v);

	void GetRange(const std::vector<FileMetaData*>& inputs,
		InternalKey* smallest,
		InternalKey* largest);

	void GetRange2(const std::vector<FileMetaData*>& inputs1,
		const std::vector<FileMetaData*>& inputs2,
		InternalKey* smallest,
		InternalKey* largest);

	void SetupOtherInputs(Compaction* c);

	// Save current contents to *log
	Status WriteSnapshot(log::Writer* log);

	void AppendVersion(Version* v);

	Env* const env_;
	const std::string dbname_;
	const Options* const options_;
	TableCache* const table_cache_;
	const InternalKeyComparator icmp_;
	uint64_t next_file_number_;
	uint64_t manifest_file_number_;
	uint64_t last_sequence_;
	uint64_t log_number_;
	uint64_t prev_log_number_;  // 0 or backing store for memtable being compacted

	// Opened lazily
	WritableFile* descriptor_file_;
	log::Writer* descriptor_log_;
	Version dummy_versions_;  // Head of circular doubly-linked list of versions.
	Version* current_;        // == dummy_versions_.prev_

	// Per-level key at which the next compaction at that level should start.
	// Either an empty string, or a valid InternalKey.
	std::string compact_pointer_[config::kNumLevels];

	// No copying allowed
	VersionSet(const VersionSet&);
	void operator=(const VersionSet&);
};

//对文件压缩的封装
class Compaction {
public:
	~Compaction();

	// Return the level that is being compacted.  Inputs from "level"
	// and "level+1" will be merged to produce a set of "level+1" files.
	int level() const { return level_; }

	// Return the object that holds the edits to the descriptor done
	// by this compaction.
	VersionEdit* edit() { return &edit_; }

	// "which" must be either 0 or 1
	int num_input_files(int which) const { return inputs_[which].size(); }

	// Return the ith input file at "level()+which" ("which" must be 0 or 1).
	FileMetaData* input(int which, int i) const { return inputs_[which][i]; }

	// Maximum size of files to build during this compaction.
	uint64_t MaxOutputFileSize() const { return max_output_file_size_; }

	// Is this a trivial compaction that can be implemented by just
	// moving a single input file to the next level (no merging or splitting)
	bool IsTrivialMove() const;

	// Add all inputs to this compaction as delete operations to *edit.
	void AddInputDeletions(VersionEdit* edit);

	// Returns true if the information we have available guarantees that
	// the compaction is producing data in "level+1" for which no data exists
	// in levels greater than "level+1".
	bool IsBaseLevelForKey(const Slice& user_key);

	// Returns true iff we should stop building the current output
	// before processing "internal_key".
	bool ShouldStopBefore(const Slice& internal_key);

	// Release the input version for the compaction, once the compaction
	// is successful.
	void ReleaseInputs();

private:
	friend class Version;
	friend class VersionSet;

	Compaction(const Options* options, int level);

	int level_;
	uint64_t max_output_file_size_;
	Version* input_version_;
	VersionEdit edit_;

	// Each compaction reads inputs from "level_" and "level_+1"
	std::vector<FileMetaData*> inputs_[2];      // The two sets of inputs

	// State used to check for number of of overlapping grandparent files
	// (parent == level_ + 1, grandparent == level_ + 2)
	std::vector<FileMetaData*> grandparents_;
	size_t grandparent_index_;  // Index in grandparent_starts_
	bool seen_key_;             // Some output key has been seen
	int64_t overlapped_bytes_;  // Bytes of overlap between current output
								// and grandparent files

	// State for implementing IsBaseLevelForKey

	// level_ptrs_ holds indices into input_version_->levels_: our state
	// is that we are positioned at one of the file ranges for each
	// higher level than the ones involved in this compaction (i.e. for
	// all L >= level_ + 2).
	size_t level_ptrs_[config::kNumLevels];
};


#endif