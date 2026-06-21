#include "string.h"

int bcmp(const void *s1, const void *s2, unsigned long n)
{
	return memcmp(s1, s2, n);
}
