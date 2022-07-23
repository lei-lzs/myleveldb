#ifndef _LEVELDB_VERSION_EDIT_HH
#define _LEVELDB_VERSION_EDIT_HH

#include "db_format.h"

#include <set>
#include <string>

class VersionSet;

struct FileMetaData {
	FileMetaData() :refs(0),allowed_seeks(1<<30),file_size(0) {

	}

	int refs;
	int allowed_seeks;
	uint64_t number; //从table文件的编号
	uint64_t file_size; //文件大小
	InternalKey smallest; //文件中Key的范围
	InternalKey largest;
};

class VersionEdit {
public:
	VersionEdit() { Clear(); }
	~VersionEdit() = delete;

	void Clear();

private:
	friend class VersionSet;

	typedef std::set<std::pair<int, uint64_t>> DeletedFileSet;

	std::string comparator_;
	uint64_t log_number_;
	uint64_t prev_log_number_;
	uint64_t next_file_number_;
	SequenceNumber last_sequence_;
	bool has_comparator_;
	bool has_log_number_;
	bool has_prev_log_number_;
	bool has_nect_file_number_;
	bool has_last_sequence_;
	
	std::vector<std::pair<int, InternalKey>> compact_pointers_;
	DeletedFileSet delete_files_;

	std::vector<std::pair<int, FileMetaData>> new_files_;

};

#endif