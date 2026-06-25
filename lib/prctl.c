/* SPDX-License-Identifier: MIT */
#include "types.h"

#define PR_GET_DUMPABLE 3

#define SUID_DUMP_DISABLE 0
#define SUID_DUMP_USER 1

int prctl(int option, ...) {

    switch(option)
    {
        case PR_GET_DUMPABLE: return SUID_DUMP_USER;
        default:  return -1;
    }
}

strong_alias(prctl, __prctl_time64)
