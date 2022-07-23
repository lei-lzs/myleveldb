#include "slice.h"
#include<string>

// Standard Put... routines append to a string
void PutFixed32(std::string* dst, uint32_t value);
void PutFixed64(std::string* dst, uint64_t value);

//往string 后面put一个value
void PutVarint32(std::string* dst, uint32_t value);
void PutVarint64(std::string* dst, uint64_t value);
void PutLengthPrefixedSlice(std::string* dst, const Slice& value);

// Standard Get... routines parse a value from the beginning of a Slice
// and advance the slice past the parsed value.
//从string前面取一个数据，并将
bool GetVarint32(Slice* input, uint32_t& value);
bool GetVarint64(Slice* input, uint64_t& value);
bool GetLengthPrefixedSlice(Slice* input, Slice* result);


void PutVarint32(std::string* dst, uint32_t value)
{
	// -- -- -- -- 按字节压缩去掉0，然后放在dst中，低位放在string的低地址
	// 七个bit为单位，第八个bit 标识是否结束
	// （1111111）			->		2^7 -1    
	// （）（）				->		2^14 -1   
	// （）（）（）			->		2^21-1    
	// （）（）（）（）		->		2^28-1  

	//每次取八位，但是编码其中的七位,将其转换为uint8_t
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

bool GetVarint32(std::string * src, uint32_t& value) 
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

//true: small endian
//false: big endian
bool EndianTest()
{
	int t = 0xFFF;

	uint8_t* p = reinterpret_cast<uint8_t*>(&t);

	uint8_t byte0 = *p;

	uint8_t byte1 = *(++p);

	return byte0 == 0xFF;
	
}