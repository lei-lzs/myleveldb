#pragma once

#ifndef _WRITABLE_FILE_HH_
#define _WRITABLE_FILE_HH_

#include "slice.h"
#include "status.h"

//A file abracting for sequential writing
class WritableFile
{
public:
	WritableFile() = default;
	WritableFile(const WritableFile&) = delete;
	WritableFile& operator = (const WritableFile&) = delete;
public:
	//添加一项
	virtual Status Append(const Slice& s) =0;

	//关闭文件
	virtual Status Close() =0;

	//缓存flush到文件
	virtual Status Flush() =0;

	//同步
	virtual Status Sync() = 0;
};


#endif
