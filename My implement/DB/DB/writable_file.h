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
	//���һ��
	virtual Status Append(const Slice& s) =0;

	//�ر��ļ�
	virtual Status Close() =0;

	//����flush���ļ�
	virtual Status Flush() =0;

	//ͬ��
	virtual Status Sync() = 0;
};


#endif
