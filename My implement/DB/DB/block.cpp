#include "block.h"
#include "coding.h"
#include "format.h"
#include "iterator.h"
#include <cassert>

Block::Block(const BlockContents& contents)
    :data_(contents.data.data())
    , size_(contents.data.size())
    ,owned_(contents.heap_allocated)
{
    if (size_ < sizeof(uint32_t)) {
        size_ = 0;
    }
    else
    {
        size_t max_restarts_allowed = (size_ - sizeof(uint32_t)) / sizeof(uint32_t);
        if (NumRestarts() > max_restarts_allowed)
        {
            size_ = 0;
        }
        else
        {
            restart_offset_ = size_ - (1 + NumRestarts()) * sizeof(uint32_t);
        }

    }
}

Block::~Block()
{
    if (owned_) {
        delete[] data_;
    }
}

Iterator* Block::NewIterator(const Comparator* comparator)
{
    return nullptr;
}

//Block的结尾记录的是重启点的长度
uint32_t Block::NumRestarts() const
{
    assert(size_ > sizeof(uint32_t));
    return DecodeFixed32(data_+size_- sizeof(uint32_t));
}


static inline const char* DecodeEntry(const char* p, const char* limit, uint32_t* shared, uint32_t* non_shared, uint32_t* value_length) {
    if (limit - p < 3) return nullptr;

    *shared = reinterpret_cast<const uint8_t*>(p)[0];
    *non_shared = reinterpret_cast<const uint8_t*>(p)[1];
    *non_shared = reinterpret_cast<const uint8_t*>(p)[2];

    //小优化，当三个长度都比较小时，可以快速读取
    if ((*shared | *non_shared | *value_length) < 128)
    {
        p += 3;
    }
    else
    {
        if ((p = GetVarint32Ptr(p, limit, shared)) == nullptr) return nullptr;
        if ((p = GetVarint32Ptr(p, limit, non_shared)) == nullptr) return nullptr;
        if ((p = GetVarint32Ptr(p, limit, value_length)) == nullptr) return nullptr;
    }
}

//Block的迭代器,对Block的内容进行解析
//DataBlock 的 restart point是稀疏的
//index block得 restart point是稠密的
class Block::Iter : public Iterator {
private:
    const Comparator* const comparator_;
    const char* const data_;
    uint32_t const restarts_; //offset of restart array 
    uint32_t const num_restarts_; //num of restarts

    uint32_t current_; //当前条目的偏移
    uint32_t restart_index_; //当前条目的重启点
    std::string key_;
    Slice value_;
    Status status_;

    inline int Compare(const Slice& a, const Slice& b) const {
        return comparator_->Compare(a, b);
    }

    inline uint32_t NextEntryOffset() const {
        return (value_.data() + value_.size()) - data_;
    }

    uint32_t GetRestartPoint(uint32_t index) {
        assert(index < num_restarts_);
        //restart数组开始位置+ 元素下标偏移位置
        return DecodeFixed32(data_ + restarts_ + index * sizeof(uint32_t));
    }

    void SeekToRestartPoint(uint32_t index) {
        key_.clear();
        restart_index_ = index;

        uint32_t offset = GetRestartPoint(index);
        
        //将value_置为重启点地址
        value_ = Slice(data_ + offset, 0);
    }

public:
    Iter(const Comparator* comparator, const char* data, uint32_t restarts,
        uint32_t num_restarts)
        :comparator_(comparator)
        , data_(data)
        , restarts_(restarts)
        , num_restarts_(num_restarts)
        , restart_index_(num_restarts)
    {
        assert(num_restarts_ > 0);
    }

    bool Valid() const override { return current_ < restarts_; }

    Status status() const override { return status_; }

    Slice key() const override {
        assert(Valid());
        return key_;
    }

    Slice value() const override {
        assert(Valid());
        return value_;
    }

    void Next() override {
        assert(Valid());
        ParseNextKey();
    }

    void Prev() override {
        assert(Valid());
        //当前位置
        const uint32_t original = current_;

        //回退到current_位置之前的 重启点
        while (GetRestartPoint(restart_index_) >= original) {
            if (restart_index_ == 0) {
                current_ = restarts_;
                restart_index_ = num_restarts_;
            }
            restart_index_--;
        }

        SeekToRestartPoint(restart_index_);

        do {
           //从重启点解析
           //注意 如果刚好是上一个  NextEntryOffset() == origianl
        } while (ParseNextKey() && NextEntryOffset() < original);
    }

    void Seek(const Slice& target) override {
        //二分查找，比target小的 最后一个restart key， 

    }

    void SeekToFirst() override
    {
        SeekToRestartPoint(0);
        ParseNextKey();
    }

    void SeekToLast() override
    {
        SeekToRestartPoint(num_restarts_-1);
        while (ParseNextKey() && NextEntryOffset() < restarts_)
        {
            //Restart point 是文件中的一些定位锚点
        }
    }

private:

    void CorruptionError() {
        current_ = restarts_;
        restart_index_ = num_restarts_;
        status_= Status::Corruption("bad entry in block");
        key_.clear();
        value_.clear();
    }

    bool ParseNextKey() {
        
        current_ = NextEntryOffset();
        const char* p = data_ + current_;
        const char* limit = data_ + restarts_;
        
        //没有entry了
        if (p > limit) {
            current_ = restarts_;
            restart_index_ = num_restarts_;
            return false;
        }
        //解析下一个entry， 
        //key 和 value的长度
        uint32_t shared, non_shared, value_length;
        p = DecodeEntry(p, limit, &shared, &non_shared, &value_length);

        //前一个key
        if (p == nullptr || key_.size() < shared) {
            CorruptionError();
            return false;
        }
        else
        {
            key_.resize(shared); //共享部分
            key_.append(p, non_shared); //不同部分
            value_ = Slice(p + non_shared, value_length);

            //current_位置的对应的重启点，最后一个小于cuurent_的
            while (restart_index_ + 1 < num_restarts_ &&
                GetRestartPoint(restart_index_ + 1) < current_) {
                ++restart_index_;
            }
            return true;
        }
    }

};
