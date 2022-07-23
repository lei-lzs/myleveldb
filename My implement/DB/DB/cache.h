#include <cstdint>

#ifndef _LEVELDB_CACHE__HH
#define _LEVELDB_CACHE__HH

class Slice;

class Cache
{
public:
	Cache() = default;

	Cache(const Cache&) = delete;
	Cache& operator=(const Cache&) = delete;

	// Destroys all existing entries by calling the "deleter"
	// function that was passed to the constructor.
	virtual ~Cache();

	//Opaque handle ,��͸����handle
	struct Handle {};

	//charge �е����Ƴ��� ������
	virtual Handle* Insert(const Slice& key, void* value, size_t charge,
		void (*deleter)(const Slice& key, void* value)) = 0;

	virtual Handle* Loolup(const Slice& key) =0;

	virtual void Release(Handle* handle)=0;

	virtual void* Value(Handle* handle)=0;

	virtual void Erase(const Slice& key) = 0;
	
	virtual uint64_t NewId() = 0;

	//���£��ͷ��ڴ�
	virtual void Prune() {}

	virtual size_t TotalCharge() const;


};


#endif
