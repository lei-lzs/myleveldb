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

//Block�Ľ�β��¼����������ĳ���
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

    //С�Ż������������ȶ��Ƚ�Сʱ�����Կ��ٶ�ȡ
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

//Block�ĵ�����,��Block�����ݽ��н���
//DataBlock �� restart point��ϡ���
//index block�� restart point�ǳ��ܵ�
class Block::Iter : public Iterator {
private:
    const Comparator* const comparator_;
    const char* const data_;
    uint32_t const restarts_; //offset of restart array 
    uint32_t const num_restarts_; //num of restarts

    uint32_t current_; //��ǰ��Ŀ��ƫ��
    uint32_t restart_index_; //��ǰ��Ŀ��������
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
        //restart���鿪ʼλ��+ Ԫ���±�ƫ��λ��
        return DecodeFixed32(data_ + restarts_ + index * sizeof(uint32_t));
    }

    void SeekToRestartPoint(uint32_t index) {
        key_.clear();
        restart_index_ = index;

        uint32_t offset = GetRestartPoint(index);
        
        //��value_��Ϊ�������ַ
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
        //��ǰλ��
        const uint32_t original = current_;

        //���˵�current_λ��֮ǰ�� ������
        while (GetRestartPoint(restart_index_) >= original) {
            if (restart_index_ == 0) {
                current_ = restarts_;
                restart_index_ = num_restarts_;
            }
            restart_index_--;
        }

        SeekToRestartPoint(restart_index_);

        do {
           //�����������
           //ע�� ����պ�����һ��  NextEntryOffset() == origianl
        } while (ParseNextKey() && NextEntryOffset() < original);
    }

    void Seek(const Slice& target) override {
        //���ֲ��ң���targetС�� ���һ��restart key�� 

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
            //Restart point ���ļ��е�һЩ��λê��
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
        
        //û��entry��
        if (p > limit) {
            current_ = restarts_;
            restart_index_ = num_restarts_;
            return false;
        }
        //������һ��entry�� 
        //key �� value�ĳ���
        uint32_t shared, non_shared, value_length;
        p = DecodeEntry(p, limit, &shared, &non_shared, &value_length);

        //ǰһ��key
        if (p == nullptr || key_.size() < shared) {
            CorruptionError();
            return false;
        }
        else
        {
            key_.resize(shared); //������
            key_.append(p, non_shared); //��ͬ����
            value_ = Slice(p + non_shared, value_length);

            //current_λ�õĶ�Ӧ�������㣬���һ��С��cuurent_��
            while (restart_index_ + 1 < num_restarts_ &&
                GetRestartPoint(restart_index_ + 1) < current_) {
                ++restart_index_;
            }
            return true;
        }
    }

};
