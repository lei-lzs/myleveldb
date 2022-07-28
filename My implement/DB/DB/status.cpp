#include "status.h"

Status::Status(const Status& s)
{
    state_ = CopyStatus(s.state_);
}

Status& Status::operator=(const Status& s)
{
    // TODO: 在此处插入 return 语句
    state_ = CopyStatus(s.state_);
    return *this;
}

Status::Status(Status&& s)
{
    delete state_;
    state_ = s.state_;
    s.state_ = nullptr;
}

Status& Status::operator=(Status&& s)
{
    // TODO: 在此处插入 return 语句
    delete state_;
    this->state_ = s.state_;
    s.state_ = NULL;
    return *this;
}

//write the lenth ,code and msg to state_
Status::Status(Code code, const Slice& msg, const Slice& msg2)
{
    assert(code != kOk);
    uint32_t msg_len = msg.size();
    uint32_t msg2_len = msg2.size();
    uint32_t size = msg_len + (msg2_len >0 ? msg2_len+2 : 0);

    char* result = new char[size + 5];

    memcpy(result, &size, sizeof(size));
    memcpy(result+4, &code, sizeof(code));
    
    memcpy(result + 5, msg.data(), msg_len);

    if (msg2_len > 0)
    {
        result[5+msg_len] = ':';
        result[6+msg_len] = ' ';
        memcpy(result + 7 + msg_len, msg2.data(), msg2_len);
    }

    state_ = result;
}

//code +msg ,eg: "NotFound: can not find the file"
std::string Status::ToString() const
{
    if (state_ == nullptr) {
        return "OK";
    }
    Code type = code();

    std::string result;
    switch (type)
    {
    case Status::kOk:
        result = "Ok:: ";
        break;
    case Status::kNotFound:
        result = "NotFound:: ";
        break;
    case Status::kCorruption:
        result = "Corruption:: ";
        break;
    case Status::kNotSupported:
        result = "NotSupported:: ";
        break;
    case Status::kInvalidArgument:
        result = "InvalidArgument:: ";
        break;
    case Status::kIOError:
        result = "IOError:: ";
        break;
    default:
        result = "Ok:: ";
        break;
    }

    uint32_t msg_len;
    memcpy(&msg_len, state_, sizeof(msg_len));

    char* ch = new char[msg_len];
    memcpy(ch, state_ + 5, msg_len);

    result.append(ch, msg_len);

    delete []ch;

    return result;
}

const char* Status::CopyStatus(const char* s)
{
    uint32_t msg_len;
    memcpy(&msg_len, s, sizeof(msg_len));

    char* result = new char[msg_len + 5]; // 5 is the header length

    memcpy(result, s, msg_len + 5);
    return result;
}
