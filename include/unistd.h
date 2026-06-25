/* SPDX-License-Identifier: MIT */

#ifndef	_UNISTD_H
#define	_UNISTD_H

#include "types.h"

#define SEEK_SET  0
#define SEEK_CUR  1
#define SEEK_END  2

ssize_t write(int fd, const void *buf, size_t count);
ssize_t read(int fd, void *buf, size_t count);
ssize_t pread(int fd, void *buf, size_t count, off_t offset);
ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset);
off_t   lseek(int fd, off_t offset, int whence);
ssize_t readblocks(int fd, off_t blockNum, size_t blockCount, void *buffer);
ssize_t writeblocks(int fd, off_t blockNum, size_t blockCount, const void *buffer);

#endif
