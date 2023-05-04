
#ifndef _LEVELDB_DB_HH
#define _LEVELDB_DB_HH

#include "status.h"

#include <string>

struct Options;
struct WriteOptions;
struct WriteBatch;
struct ReadOptions;
class Iterator;

//db.h是暴漏给用户的头文件，里面定义了DB提供的操作接口
// Put,Delete,Write,Get,Iterator,SnapShot, 全部为纯虚接口
//采取Imp类实现方法

class DB {
public:

	//打开一个DB
	static Status Open(const Options& options,
		const std::string& name,
		DB** dpptr);

	DB() = default;

	DB(const DB&) = delete;
	DB& operator =(const DB&) = delete;

	virtual ~DB();

	virtual Status Put(const WriteOptions& options,
					   const Slice& key,
					   const Slice& value) =0 ;

	virtual Status Delete(const WriteOptions& options, const Slice& key) = 0;

	virtual Status Write(const WriteOptions& options, WriteBatch* updates) = 0;

	virtual Status Get(const ReadOptions& options,
		const Slice& key, std::string* value) = 0;

	virtual Iterator* NewIterator(const ReadOptions& options) = 0;
};

#endif
