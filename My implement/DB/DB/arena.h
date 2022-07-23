#ifndef _UTIL_ARENA_HH_
#define _UTIL_ARENA_H_

#include <vector>
#include <atomic>

#define kBlockSize 4096

class Arena
{
public:
	Arena();
	Arena(const Arena&) = delete;
	Arena& operator= (const Arena&) = delete;

	~Arena();

	//allocate n bytes memory.
	char* Allocate(size_t n);

	//allocate n bytes memory by the way of alignment.
	char* AllocateAligned(size_t n);

	//estimate of the total memory usage allocated by arena
 	size_t MemoryUsage() const
	{
		return memory_usage_.load(std::memory_order_relaxed);
	}
private:
	char* AllocateFallback(size_t n);
	char* AllocateNewBlock(size_t n);

	//allocation state
	char* alloc_ptr_;
	size_t alloc_bytes_remaining_;

	//array of memory blocks allocated by arena
	std::vector<char*> blocks_;

	std::atomic<size_t> memory_usage_;
};

inline char* Arena::Allocate(size_t n)
{
	//remaining is enough, alloc from the block
	if (n < alloc_bytes_remaining_)
	{
		char* result = alloc_ptr_;
		alloc_ptr_ += n;
		alloc_bytes_remaining_ -= n;
		return result;
	}
	//otherwise , should consider a new block 
	return AllocateFallback(n);
}

#endif
