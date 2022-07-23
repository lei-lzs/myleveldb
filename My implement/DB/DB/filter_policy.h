#ifndef _LEVELDB_FILTER_POLICY_HH
#define _LEVELDB_FILTER_POLICY_HH

#include <string>

class Slice;

class FilterPolicy
{
public:
	virtual ~FilterPolicy();

	//策略的名称
	virtual const char* Name() const = 0;

	//Keys中记录了[0,n-1]个key， 将其Filter数据存到dst中.
	virtual void CreateFilter(const Slice& keys, int n, std::string* dst) const = 0;

	//如果过滤器匹配上了，返回true，则key大概率存在，如果过滤器不匹配，则返回false，则key一定不存在
	virtual bool KeyMayMatch(const Slice& key, const Slice& filter) const = 0;

	const FilterPolicy* NewBloomFiterPolicy(int bits_per_key);

};

#endif
