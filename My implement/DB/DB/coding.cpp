#include "coding.h"

inline void PutFixed32(std::string* dst, uint32_t value)
{
	for (int i = 0; i < 4; i++) {
		value >> (8 * i);
		unsigned char ch = *reinterpret_cast<unsigned char*>(&value);
		dst->push_back(ch);
	}

}
void PutFixed64(std::string* dst, uint64_t value) {
	
	char ch[8];
	EncodeFixed64(ch,value);
	dst->append(ch, sizeof(ch));
}

void PutVarint32(std::string* dst, uint32_t value)
{
	// -- -- -- -- 按字节压缩去掉0，然后放在dst中，低位放在string的低地址
	// 七个bit为单位，第八个bit 标识是否结束，0则结束，1未结束
	// （1111111）			->		2^7 -1    
	// （）（）				->		2^14 -1   
	// （）（）（）			->		2^21-1    
	// （）（）（）（）		->		2^28-1  

	//每次取八位，但是编码其中的七位,将其转换为uint8_t

	//如果是单个的数，可以不需要第8位标志是否结束，但是一个string存储多个变长数，就需要标记位
	//来解析实际的数。
	uint8_t byte;
	uint8_t B = 128;

	if (value < (1 << 7)) {
		byte = value;
		dst->push_back(byte);
	}
	else if (value < (1 << 14)) {
		byte = value;
		byte |= B;
		dst->push_back(byte);

		value >>= 7;
		byte = value;
		dst->push_back(byte);
	}
	else if (value < (1 << 21)) {
		byte = value;
		byte |= B;
		dst->push_back(byte);

		value >>= 7;
		byte = value;
		byte |= B;
		dst->push_back(byte);

		value >>= 7;
		byte = value;
		dst->push_back(byte);
	}
	else if (value < (1 << 28)) {
		byte = value;
		byte |= B;
		dst->push_back(byte);

		value >>= 7;
		byte = value;
		byte |= B;
		dst->push_back(byte);

		value >>= 7;
		byte = value;
		byte |= B;
		dst->push_back(byte);

		value >>= 7;
		byte = value;
		dst->push_back(byte);
	}

}

void PutVarint64(std::string* dst, uint64_t value)
{
	char buffer[8];

	char* re = EncodeVarint64(buffer, value);
	dst->append(buffer, re - buffer);
}

void PutLengthPrefixedSlice(std::string* dst, const Slice& value)
{
	PutVarint32(dst, value.size());
	dst->append(value.data(), value.size());
}

bool GetVarint32(Slice* input, uint32_t& value)
{
	//按Byte读取，直到Byte高位为0，结束
	const char* p = input->data();
	const char* limit = p + input->size();
	const char* q = GetVarint32Ptr(p, limit, &value);

	if (q == nullptr) {
		return false;
	}
	else {
		*input = Slice(q, limit - q);
		return true;
	}

	return false;
}

bool GetVarint64(Slice* input, uint64_t& value)
{
	return false;
}

bool GetLengthPrefixedSlice(Slice* input, Slice* result)
{
	uint32_t len;
	if (GetVarint32(input, len) && input->size() >= len) { //读取长度信息
		*result = Slice(input->data(), len); //读取Slice
		input->remove_prefix(len);  // remove 已经读取的部分，注意，只有gGetVarint的时候，slice才会自动向后偏移。
		return true;
	}
	else
	{
		return false;
	}
}

//64位和32位为何要不一样的实现，32位的为何要有FallBack处理？
const char* GetVarint32Ptr(const char* p, const char* limit, uint32_t* v)
{
	if (p < limit) {
		uint32_t result = *reinterpret_cast<const uint8_t*>(p);
		if ((result & 128) == 0) // result 第八位为0，结束，只有一个Byte,特殊处理
		{
			*v = result;
			return p + 1;
		}
	}
	return GetVarint32PtrFallBack(p,limit,v);
}

const char* GetVarint64Ptr(const char* p, const char* limit, uint64_t* v)
{
	uint64_t result = 0;
	for (uint32_t shift = 0; shift <= 28 && p < limit; shift += 7) {
		uint64_t byte = *reinterpret_cast<const uint8_t*>(p);
		p++;

		if (byte & 128) {
			result |= ((byte & 127) << shift);  //高位置为0
		}
		else { //最后一字节不需要处理
			result |= (byte << shift);
			*v = result;
			return reinterpret_cast<const char*>(p);
		}
	}
	return nullptr;
}

bool GetVarint32(std::string* src, uint32_t& value)
{
	uint8_t B = 128;
	int size = src->size();
	uint32_t cur_v = 0;

	for (int i = 0; i < size; i++)
	{
		uint8_t v = *reinterpret_cast<uint8_t*>(&(*src)[i]);
		cur_v = v & ~B;// &0xFF >> 1;

		cur_v <<= (7 * i);
		value |= cur_v;
	}

	return true;
}

int VarintLength(uint64_t v)
{
	int len = 0;
	while (v >= 128)
	{
		len++;
		v = (v >> 7);
	}
	return len+1;
}

char* EncodeVarint32(char* dst, uint32_t v)
{
	uint8_t* ptr = reinterpret_cast<uint8_t*>(dst);

	static const int B = 128; // 10000000  

	if (v < (1 << 7)) {
		*(ptr++) = v;
	}
	else if (v < (1 << 14)) {
		*(ptr++) = v | B;
		*(ptr++) = v >> 7;
	}
	else if (v < (1 << 21)) {
		*(ptr++) = v | B;
		*(ptr++) = (v >> 7) | B;
		*(ptr++) = (v >> 14);
	}
	else if (v < (1 << 28)) {
		*(ptr++) = v | B;
		*(ptr++) = (v >> 7) | B;
		*(ptr++) = (v >> 14) |B;
		*(ptr++) = v >> 21;
	}
	else {
		*(ptr++) = v | B;
		*(ptr++) = (v >> 7) | B;
		*(ptr++) = (v >> 14) | B;
		*(ptr++) = (v >> 21) | B;
		*(ptr++) = v >> 28;
	}

	return reinterpret_cast<char*>(ptr);
}

char* EncodeVarint64(char* dst, uint64_t v)
{
	uint8_t* ptr = reinterpret_cast<uint8_t*>(dst);
	static const int  B = 128;
	while (v >= B)
	{
		*(ptr++) = v | B;
		v= (v >> 7);
	}

	*(ptr++) = v;

	return reinterpret_cast<char*>(ptr);

	return nullptr;
}

void EncodeFixed32(char* dst, uint32_t value)
{
	uint8_t* buffer = reinterpret_cast<uint8_t*>(dst);

	buffer[0] = value;
	buffer[1] = value >> 8;
	buffer[2] = value >> 16;
	buffer[3] = value >> 24;
}

void EncodeFixed64(char* dst, uint32_t value)
{
	uint8_t* buffer = reinterpret_cast<uint8_t*>(dst);
	for (int i = 0; i < 8; i++) {		
		buffer[i] = value >> (8*i);
	}
	
}

const char* GetVarint32PtrFallBack(const char* p, const char* limit, uint32_t* value)
{
	uint32_t result = 0;
	for (uint32_t shift = 0; shift <= 28 && p < limit; shift += 7) {
		uint32_t byte = *reinterpret_cast<const uint8_t*>(p);
		p++;

		if (byte & 128) {
			result |= ((byte & 127) << shift);  //高位置为0
		}
		else { //最后一字节不需要处理
			result |= (byte << shift);
			*value = result;
			return reinterpret_cast<const char*>(p);
		}
	}
	return nullptr;
}
