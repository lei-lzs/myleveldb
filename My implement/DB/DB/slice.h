#pragma once
#ifndef _SILCE_HH
#define _SILCE_HH
#include <cassert>
#include <string>

class Slice
{
public:
    Slice() { data_ = nullptr; size_ = 0; };
    Slice(const char* d, size_t s): data_(d),size_(s) {}
    Slice(const std::string& s) { data_ = s.data(); size_ = s.size(); }
    Slice(const char* s) { data_ = s; size_ = strlen(s); }
    
    Slice(const Slice& other) = default;
    Slice& operator=(const Slice& other) = default;

    // should not have non-trtival destructor as it's just a view proxy.
    ~Slice() =  default;
public:
    const char* data() const { return data_; }
    size_t size() const { return size_; }   
    bool empty() const { return size_ == 0; }

    char operator[](size_t n) { 
        assert(n <size_);
        return data_[n]; 
    }
    void clear() {
        data_ = nullptr;
        size_ = 0;
    }
    void remove_prefix(size_t n) {
        while (n--) {
            data_++;
            size_--;
        }
    }
    std::string ToString() {
        return std::string(data_, size_);
    }

    //*this < s return -1
    //*this > s return 1
    //*this == s return 0; 
    int compare( Slice& s) {
        size_t min_size = size() <= s.size() ? size() : s.size();
        int r = memcmp(data(), s.data(), min_size);

        if (r == 0) {
            if (size() > s.size()) {
                return 1;
            }
            if (size() < s.size()) {
                return -1;
            }
        }

        return r;
    }

    bool starts_with(const Slice& s) {
        return s.size() <= size() && memcmp(data(), s.data(), s.size());
    }

private:
    const char* data_;
    size_t size_;

};

inline bool operator == (const Slice& a, const Slice& b)
{
    return a.size() == b.size() && memcmp(a.data(), b.data(),a.size());
}

inline bool operator != (const Slice& a, const Slice& b) 
{
    return !(a == b);
}


#endif
