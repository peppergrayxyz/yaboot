/* we got no time */
long time()
{
    return 0;
}

typedef long long yaboot_time64_t;

struct yaboot_timeval64 {
	yaboot_time64_t tv_sec;
	long tv_usec;
};

yaboot_time64_t __time64(yaboot_time64_t *t)
{
	if (t)
		*t = 0;
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
