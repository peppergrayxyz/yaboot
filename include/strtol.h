#ifndef __STRTOL_H
#define __STRTOL_H

unsigned long long strtox(const char *restrict s, char **restrict p, int base, unsigned long long lim);
unsigned long long strtoull(const char *restrict s, char **restrict p, int base);
long long strtoll(const char *restrict s, char **restrict p, int base);
unsigned long strtoul(const char *restrict s, char **restrict p, int base);
long strtol(const char *restrict s, char **restrict p, int base);

#endif
