/* SPDX-License-Identifier: MIT */

#ifndef	_FCNTL_H
#define	_FCNTL_H

#include "types.h"

#define O_RDONLY  00
#define O_WRONLY  01
#define O_RDWR    02

int open(const char *path, int flags);
int close(int fd);

#endif