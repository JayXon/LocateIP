#ifndef __UTIL_H_
#define __UTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "ipdb.h"

char* ip2str(char *buf, size_t len, int ip);

uint32_t str2ip(const char *lp);

#ifdef __cplusplus
}
#endif

#endif // __UTIL_H_
