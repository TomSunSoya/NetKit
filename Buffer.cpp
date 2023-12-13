//
// Created by qiuyudai on 2023/12/13.
//

#include <sys/uio.h>
#include <cerrno>
#include "Buffer.h"

const char Buffer::kCRLF[] = "\r\n";

const size_t Buffer::kCheapPrepend;
const size_t Buffer::kInitialSize;

ssize_t Buffer::readFd(int fd, int *savedErrno) {
    char extra_buf[65536];
    struct iovec vec[2];
    const auto writeable = writableBytes();
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writeable;
    vec[1].iov_base = extra_buf;
    vec[1].iov_len = sizeof extra_buf;

    const auto n = readv(fd, vec, 2);

    if (n < 0)
        *savedErrno = errno;
    else if (static_cast<size_t>(n) <= writeable)
        writerIndex_ += n;
    else {
        writerIndex_ = buffer_.size();
        append(extra_buf, n - writeable);
    }
}
