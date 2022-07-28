#include "slice.h"
#include<string>

// Standard Put... routines append to a string
void PutFixed32(std::string* dst, uint32_t value);
void PutFixed64(std::string* dst, uint64_t value);

//往string 后面put一个value
void PutVarint32(std::string* dst, uint32_t value);
void PutVarint64(std::string* dst, uint64_t value);

//(Vari)Length + Silce
void PutLengthPrefixedSlice(std::string* dst, const Slice& value);

// Standard Get... routines parse a value from the beginning of a Slice
// and advance the slice past the parsed value.
//从string前面取一个数据，并将
bool GetVarint32(Slice* input, uint32_t& value);
bool GetVarint64(Slice* input, uint64_t& value);
bool GetLengthPrefixedSlice(Slice* input, Slice* result);

//解析一个Int32，并返回Slice的结束位置
const char* GetVarint32Ptr(const char* p, const char* limit, uint32_t* v);
const char* GetVarint64Ptr(const char* p, const char* limit, uint64_t* v);

//the encoding length of v  [1:8]
int VarintLength(uint64_t v);

char* EncodeVarint32(char* dst, uint32_t value);
char* EncodeVarint64(char* dst, uint64_t value);

void EncodeFixed32(char* dst, uint32_t value);
void EncodeFixed64(char* dst, uint32_t value);

uint32_t DecodeFixed32(const char* dst);
uint64_t DecodeFixed64(const char* dst);

const char* GetVarint32PtrFallBack(const char* p, const char* limit, uint32_t* value);


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