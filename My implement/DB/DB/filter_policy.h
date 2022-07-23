#ifndef _LEVELDB_FILTER_POLICY_HH
#define _LEVELDB_FILTER_POLICY_HH

#include <string>

class Slice;

class FilterPolicy
{
public:
	virtual ~FilterPolicy();

	//���Ե�����
	virtual const char* Name() const = 0;

	//Keys�м�¼��[0,n-1]��key�� ����Filter���ݴ浽dst��.
	virtual void CreateFilter(const Slice& keys, int n, std::string* dst) const = 0;

	//���������ƥ�����ˣ�����true����key����ʴ��ڣ������������ƥ�䣬�򷵻�false����keyһ��������
	virtual bool KeyMayMatch(const Slice& key, const Slice& filter) const = 0;

	const FilterPolicy* NewBloomFiterPolicy(int bits_per_key);

};

#endif
