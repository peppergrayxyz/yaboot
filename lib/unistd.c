/* SPDX-License-Identifier: MIT */

#include "unistd.h"
#include "fcntl.h"
#include "prom.h"
#include "stdlib.h"

int getpid(void) {
    return 0;
}

int getuid(void) {
    return 0;
}

int geteuid(void) {
    return 0;
}

int getgid(void) {
    return 0;
}

int getegid(void) {
    return 0;
}

int gethostname(char *name, size_t size) {
    if(size > 0) name[0] = '\0';
    return 0;
}

typedef struct {
    ihandle  handle;
    off_t    offset;
    uint32_t blksize;
} fd_t;

int open(const char *pathname, int flags) 
{
    (void) flags;

    prom_handle handle = prom_open((char *)pathname); 
    if(handle < 0) return -1;

    fd_t *_fd = malloc(sizeof(fd_t));
    if(!_fd) {
        prom_close(handle);
        return -1;
    }

    off_t blksize = prom_getblksize(_fd->handle);
    if (blksize <= 1) blksize = 512;

    _fd->handle  = handle;
    _fd->offset  = 0;
    _fd->blksize = blksize; 

    return (int) _fd;
}

int close(int fd) 
{
    if(!fd) return -1;

    fd_t *_fd = (fd_t*) fd;
    
    prom_close(_fd->handle);
    free(_fd);
    return 0;
}

static off_t unistd_seek(int fd, off_t offset) 
{
    if(!fd) return -1;
    fd_t *_fd = (fd_t*) fd;

    off_t status = prom_lseek(_fd->handle, offset);
    if (status != offset) {
        prom_printf("Can't seek %x to %x%32x\n", _fd->handle,
				  (uint32_t)(offset >> 32), (uint32_t)(offset & 0xffffffffUL));
        return -1;
    }
    return status;
}

off_t lseek(int fd, off_t offset, int whence) 
{
    if(!fd) return -1;
    fd_t *_fd = (fd_t*) fd;

    off_t location;
    off_t size;

    switch(whence)
    {
        case SEEK_SET:  location = offset;              break;
        case SEEK_CUR:  location = _fd->offset + offset; break;
        case SEEK_END:  if((size = prom_getfilesize(_fd->handle)) < 0) return -1;
                        location = size + offset;       break;
        default:        return -1;
    }

    if(unistd_seek(_fd->handle, location) != 0) return -1;
    _fd->offset = location;

    return location;
}

typedef ssize_t (prom_io_t)(ihandle file, void *buf, off_t len);

static ssize_t unistd_rw(prom_io_t prom_rw, int fd, void *buf, size_t count)
{
    if(!fd || !buf) return -1;

    fd_t *_fd = (fd_t*) fd;
    ssize_t num_bytes = prom_rw(_fd->handle, buf, count);
    if(num_bytes < 0) return -1;
    _fd->offset += num_bytes;
    return num_bytes;

}

ssize_t read(int fd, void *buf, size_t count) {
    return unistd_rw(prom_read, fd, buf, count);
}

ssize_t write(int fd, const void *buf, size_t count) {
    return unistd_rw(prom_write, fd, (void *) buf, count);
}

static ssize_t unistd_prw(prom_io_t prom_rw, int fd, void *buf, size_t count, off_t offset)
{
    if(!fd || !buf) return -1;
    fd_t *_fd = (fd_t*) fd;  

    off_t status = unistd_seek(_fd->handle, offset);
    if (status != offset) return -1;

    off_t procd = prom_rw(_fd->handle, buf, count);
    if(procd < 0) return -1;  

    return procd;
}

ssize_t pread(int fd, void *buf, size_t count, off_t offset) {
    return unistd_prw(prom_read, fd, buf, count, offset);
}

ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset) {
    return unistd_prw(prom_write, fd, (void *) buf, count, offset);
}

static ssize_t unistd_rwblocks (prom_io_t prom_rw, int fd, off_t blockNum, size_t blockCount, void *buf)
{    
    if(!fd || !buf) return -1;
    fd_t *_fd = (fd_t*) fd;  

    off_t blksize = _fd->blksize;
    off_t offset  = blockNum * blksize;
    off_t count   = blockNum * blockCount; 
    int procd     = unistd_prw(prom_rw, fd, buf, count, offset);
       
    return  (procd < 0) ? -1 : procd / blksize;
}

ssize_t readblocks(int fd, off_t blockNum, size_t blockCount, void *buf) {
    return unistd_rwblocks (prom_read, fd, blockNum, blockCount, buf);
}

ssize_t writeblocks(int fd, off_t blockNum, size_t blockCount, const void *buf) {
    return unistd_rwblocks (prom_read, fd, blockNum, blockCount, (void *) buf);
}

