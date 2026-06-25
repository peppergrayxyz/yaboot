#include "types.h"
#include "prom.h"

typedef __s64 yaboot_time64_t;

struct yaboot_timeval64 {
	yaboot_time64_t tv_sec;
	__s64 tv_usec;
};

struct yaboot_timezone {
    int tz_minuteswest; /* minutes west of Greenwich */
    int tz_dsttime;     /* type of DST correction */
};

/* we got no time */
yaboot_time64_t time(yaboot_time64_t *t)
{
	if (t) *t = 0;
	return 0;
}

int __gettimeofday_time64(struct yaboot_timeval64 *tv, void *tz)
{
	(void) tz;

	if (tv) {
		tv->tv_sec = 0;
		tv->tv_usec = 0;
	}
	return 0;
}

int __stat_time64(const char *path, void *st)
{
	(void) path;
	(void) st;
	return -1;
}

int __fstat_time64(int fd, void *st)
{
	(void) fd;
	(void) st;
	return -1;
}

int __gettimeofday64(struct yaboot_timeval64 *restrict tv, struct yaboot_timezone *restrict tz) {
	return 0;
}

void abort(void) {
	prom_abort("abort() called");
}

void __fortify_fail (const char *msg) {
	prom_abort("%s\n", msg);
}

void __stack_chk_fail(void) {
	__fortify_fail("stack smashing detected");
}

strong_alias(time, __time64)
strong_alias(__stat_time64, __stat64_time64)
strong_alias (__stack_chk_fail, __stack_chk_fail_local)

