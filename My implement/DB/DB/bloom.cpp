#include "filter_policy.h"
#include "slice.h"
#include "hash.h"

static uint32_t BloomHash(const Slice& key) {
	return Hash(key.data(), key.size(), 0xbc9f1d34);
}


class BloomFilterPolicy : public FilterPolicy
{
private:
	size_t bits_per_key_;
	size_t k_;
public:

	//计算hash函数的个数
	explicit BloomFilterPolicy(int bitp_per_key)
		:bits_per_key_(bitp_per_key) {
		k_ = static_cast<size_t>(bitp_per_key * 0.69); // ln2
		if (k_ < 1) k_ = 1;
		if (k_ > 30) k_ = 30;
	}

	virtual const char* Name() const {
		return "leveldb.BuiltinBloomFilter2";
	}

	virtual void CreateFilter(const Slice* keys, int n, std::string* dst) const {
		//keys key数组， n key的个数  ，dst生成的 bloom map
		size_t bits = n * bits_per_key_;
		if (bits < 64) bits = 64;

		//bytes 数
		size_t bytes = (bits + 7) / 8;
		bits = bytes * 8;

		const size_t init_size = dst->size();
		dst->resize(init_size + bytes, 0);
		dst->push_back(static_cast<char>(k_));
		char* array = &(*dst)[init_size];

		for (int i = 0; i < n; i++)
		{
			uint32_t h = BloomHash(keys[i]);
			const uint32_t delta = (h >> 17) | (h << 15); //[低17位][高15位]
			for (size_t j = 0; j < k_; j++)
			{
				const uint32_t bitpos =h % bits;
				array[bitpos / 8] |= (1 << bitpos % 8); //将目标为置为1
				h += delta;
			}
		}

	}

	virtual bool KeyMayMatch(const Slice& key, const Slice& bloom_filter) const {
		//读取最后一个byte 的k_
		//计算key的hash位，看是不是都为1， 不为1 就返回false
		uint32_t bytes = bloom_filter.size();

		const char* array = bloom_filter.data();
		size_t k = array[bytes-1];

		if (k > 30) {
			return true;
		}
	
		uint32_t bits = bytes * 8 - 8;

		//相当于把hash再重新hash几次，得到多个hash值
		//不同的key， hash到相同的多个bit位的概率？？
		uint32_t h = BloomHash(key);
		const uint32_t delta = (h >> 17) | (h << 15); //[低17位][高15位]
		for (size_t j = 0; j < k; j++)
		{
			const uint32_t bitpos = h % bits;
			if ((array[bitpos / 8] & (1 << bitpos % 8)) != 1) //检查目标为置是否为1
			{
				return false;
			}
			h += delta;
		}
		return true;
	}
};


const FilterPolicy* NewBloomFilterPolicy(int bit_per_key)
{
	return new BloomFilterPolicy(bit_per_key);
}