#include "arena.h"

char* Arena::AllocateFallback(size_t n)
{
	if (n > kBlockSize / 4) {

		//allocate a real size required by n
		char* result = AllocateNewBlock(n);
		return result;
	}
	
	// allocate a new fullsize block, wash the left memory of previous block.
	char* result = AllocateNewBlock(kBlockSize);
	alloc_ptr_ = result+ n;
	alloc_bytes_remaining_ = kBlockSize-n;

	return result;
}

char* Arena::AllocateNewBlock(size_t n)
{
	char* ptr = new char[n];
	blocks_.push_back(ptr);

	memory_usage_.fetch_add(n + sizeof(char*), std::memory_order_relaxed);
}