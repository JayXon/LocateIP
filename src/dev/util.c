#include "ipdb.h"

char* ip2str(char *buf, size_t len, int ip)
{
    buf[--len] = 0;

    int i=0;
    for(i=0;i<4;i++)
    {
        int dec = ip & 0xFF;
        do
        {
            buf[--len] = dec%10 + '0';
            dec/=10;
        }while(dec!=0);

        if(i<3) buf[--len] = '.';
        ip = ip>>8;
    }
    return buf + len;
    //sprintf(buf, "%d.%d.%d.%d", (uint8_t)(ip>>24), (uint8_t)(ip>>16), (uint8_t)(ip>>8), (uint8_t)ip);
}

uint32_t str2ip(const char *lp)
{
    uint32_t ret = 0;
    uint8_t now = 0;

    while(*lp)
    {
        if('.' == *lp)
        {
            ret = 256 * ret + now;
            now = 0;
        }
        else
        {
            now = 10 * now + *lp - '0';
        }

        ++lp;
    }
    ret = 256 * ret + now;

    return ret;
}


buffer* buffer_create()
{
    return calloc(1, sizeof(buffer));
}

uint32_t buffer_expand(buffer *buf, uint32_t length)
{
    buf->size += length;
    if(buf->size>buf->capacity)
    {
        buf->capacity = (buf->capacity + length)*3/2;
        buf->data = (uint8_t*)realloc(buf->data, buf->capacity);
    }
    return buf->size - length;
}

uint32_t buffer_append(buffer *buf, const void* src, uint32_t length)
{
    uint32_t offset = buffer_expand(buf, length);
    memcpy(buf->data + offset, src, length);
    return offset;
}

uint8_t* buffer_get(const buffer *buf)
{
    return buf->data;
}

uint32_t buffer_size(const buffer *buf)
{
    return buf->size;
}

void buffer_release(buffer *buf)
{
    if(buf->data) free(buf->data);
    free(buf);
}
