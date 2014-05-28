#ifndef __UTIL_H_
#define __UTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "ipdb.h"

char* ip2str(char *buf, size_t len, int ip);
uint32_t str2ip(const char *lp);

// memory buffer
typedef struct
{
    uint8_t*        data;
    uint32_t        size;
    uint32_t        capacity;
} buffer;

buffer* buffer_create();
uint32_t buffer_expand(buffer *buf, uint32_t length);
uint32_t buffer_append(buffer *buf, const void* src, uint32_t length);
uint8_t* buffer_get(const buffer *buf);
uint32_t buffer_size(const buffer *buf);
void buffer_release(buffer *buf);

#ifdef __cplusplus
}
#endif

#endif // __UTIL_H_
