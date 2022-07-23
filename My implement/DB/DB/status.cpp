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

std::string Status::ToString() const
{
    return std::string();
}

const char* Status::CopyStatus(const char* s)
{
    uint32_t msg_len;
    memcpy(&msg_len, s, sizeof(msg_len));

    char* result = new char[msg_len + 5]; // 5 is the header length

    memcpy(result, s, msg_len + 5);
    return result;
}
