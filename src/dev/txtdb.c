#include "ipdb.h"
#include <ctype.h>

uint32_t skip_linefeed(const uint8_t *buffer, uint32_t length)
{
    uint32_t offset = 0;
    for(;offset<length;offset++)
    {
        if(buffer[offset]!='\r' && buffer[offset]!='\n') break;
    }
    return offset;
}
uint32_t find_linefeed(const uint8_t *buffer, uint32_t length, char *buf)
{
    uint32_t offset = 0;
    for(;offset<length;offset++)
    {
        if(buffer[offset]=='\r' || buffer[offset]=='\n')
        {
            memcpy(buf, buffer, offset);
            buf[offset] = '\0';
            return offset;
        };
    }
    return 0;
}
uint32_t readline(const uint8_t *buffer, uint32_t length, char *buf)
{
    uint32_t skip = skip_linefeed(buffer, length);
    uint32_t offset = find_linefeed(buffer + skip, length - skip, buf);
    if(offset)
    {
        return offset + skip;
    }
    return 0;
}

bool is_legal_ip(const char *ip)
{
    uint8_t i = 0;
    uint16_t n = 0;
    while(*ip)
    {
        if(*ip=='.')
        {
            i++;
            n = 0;
        }
        else if(isdigit(*ip))
        {
            n = n * 10 + *ip - '0';
            if(n>255) return false;
        }
        else
        {
            //return false;
        }
        ip++;
    }
    return i==3;
}

bool split_line(char *buf, char **lower, char **upper, char **zone, char **area)
{
    uint8_t state = 1;
    char *ptr = buf;

    while(*ptr)
    {
        switch(state)
        {
        case 1:
            if(!isspace(*ptr))
            {
                *lower = ptr;
                state = 2;
            }
            break;
        case 2:
            if(isspace(*ptr))
            {
                *ptr = 0;
                state = 3;
            }
            break;
        case 3:
            if(!isspace(*ptr))
            {
                *upper = ptr;
                state = 4;
            }
            break;
        case 4:
            if(isspace(*ptr))
            {
                *ptr = 0;
                state = 5;
            }
            break;
        case 5:
            if(!isspace(*ptr))
            {
                *zone = ptr;
                state = 6;
            }
            break;
        case 6:
            if(isspace(*ptr))
            {
                *ptr = 0;
                *area = ptr + 1;
                state = 7;
            }
            break;
        }
        ptr++;
    }

    if(!is_legal_ip(*lower)) return false;
    if(!is_legal_ip(*upper)) return false;

    if(state==6)
    {
        state = 7;
        *zone = *zone - 1;
        *area = "";
    }
    return state==7;
}

static bool txtdb_iter(const ipdb *ctx, ipdb_item *item, uint32_t index)
{
    static char buf[1024];
    static uint32_t last_offset = 0;
    if(index==0) last_offset = 0;

    uint32_t offset = 0;
    while((offset = readline(ctx->buffer + last_offset, ctx->length - last_offset, buf)))
    {
        last_offset += offset;
        char *lower, *upper, *zone, *area;
        bool right = split_line(buf, &lower, &upper, &zone, &area);
        if(right)
        {
            item->lower = str2ip(lower);
            item->upper = str2ip(upper);
            item->zone = zone;
            item->area = area;
            return true;
        }
    }
    return false;
}

static bool txtdb_find(const ipdb *ctx, ipdb_item *item, uint32_t ip)
{
    return false;
}

static bool txtdb_init(ipdb * ctx, const uint8_t *buffer, uint32_t length)
{
    ctx->buffer = buffer;
    ctx->length = length;

    char buf[1024];
    uint32_t last_offset = 0;
    uint32_t offset = 0;
    while((offset = readline(buffer + last_offset, length - last_offset, buf)))
    {
        last_offset += offset;
        char *lower, *upper, *zone, *area;
        bool right = split_line(buf, &lower, &upper, &zone, &area);
        if(right) ctx->count++;
    }

    return ctx->count!=0;
}

const ipdb_handle txtdb_handle = {txtdb_init, txtdb_iter, txtdb_find};
