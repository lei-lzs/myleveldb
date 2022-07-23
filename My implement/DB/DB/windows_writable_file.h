#pragma once
#ifndef _WINDOWS_WRITABLE_FILE_HH
#define _WINDOWS_WRITABLE_FILE_HH

#include "writable_file.h"

class WindowsWritableFile : public WritableFile {
public:
    WindowsWritableFile(std::string filename, ScopedHandle handle)
        : pos_(0), handle_(std::move(handle)), filename_(std::move(filename)) {}

    ~WindowsWritableFile() override = default;

    Status Append(const Slice& data) override {
        size_t write_size = data.size();
        const char* write_data = data.data();

        // Fit as much as possible into buffer.
        size_t copy_size = std::min(write_size, kWritableFileBufferSize - pos_);
        std::memcpy(buf_ + pos_, write_data, copy_size);
        write_data += copy_size;
        write_size -= copy_size;
        pos_ += copy_size;
        if (write_size == 0) {
            return Status::OK();
        }

        //先写buffer,buffer写的下就返回，写不下，先把buffer flush一下，然后再把剩余尝试写buffer。

        // Can't fit in buffer, so need to do at least one write.
        Status status = FlushBuffer();
        if (!status.ok()) {
            return status;
        }

        // Small writes go to buffer, large writes are written directly.
        if (write_size < kWritableFileBufferSize) {
            std::memcpy(buf_, write_data, write_size);
            pos_ = write_size;
            return Status::OK();
        }
        return WriteUnbuffered(write_data, write_size);
    }

    Status Close() override {
        Status status = FlushBuffer();
        if (!handle_.Close() && status.ok()) {
            status = WindowsError(filename_, ::GetLastError());
        }
        return status;
    }

    Status Flush() override { return FlushBuffer(); }

    Status Sync() override {
        // On Windows no need to sync parent directory. Its metadata will be updated
        // via the creation of the new file, without an explicit sync.

        Status status = FlushBuffer();
        if (!status.ok()) {
            return status;
        }

        if (!::FlushFileBuffers(handle_.get())) {
            return Status::IOError(filename_,
                GetWindowsErrorMessage(::GetLastError()));
        }
        return Status::OK();
    }

private:
    Status FlushBuffer() {
        Status status = WriteUnbuffered(buf_, pos_);
        pos_ = 0;
        return status;
    }

    Status WriteUnbuffered(const char* data, size_t size) {
        DWORD bytes_written;
        if (!::WriteFile(handle_.get(), data, static_cast<DWORD>(size),
            &bytes_written, nullptr)) {
            return Status::IOError(filename_,
                GetWindowsErrorMessage(::GetLastError()));
        }
        return Status::OK();
    }

    // buf_[0, pos_-1] contains data to be written to handle_.
    char buf_[kWritableFileBufferSize];
    size_t pos_;

    ScopedHandle handle_;
    const std::string filename_;
};


#endif