#include "ipdb.h"

// http://www.cz88.net/

inline static uint32_t get_3b(const char *mem)
{
    return 0x00ffffff & *(uint32_t*)(mem);
}

static bool qqwry_iter(const ipdb *ctx, ipdb_item *item, uint32_t index)
{
    if(index<ctx->count)
    {
        char *ptr = (char*)ctx->buffer;
        char *p = ptr + *(uint32_t*)ptr;

        uint32_t temp = get_3b(p + 7 * index + 4);

        item->lower = *(uint32_t*)(p + 7 * index);
        item->upper = *(uint32_t*)(ptr + temp);

        char *offset = ptr + temp + 4;

        if( 0x01 == *offset )
            offset = ptr + get_3b(offset + 1);

        if( 0x02 == *offset )
        {
            item->zone = (const char *)( ptr + get_3b(offset + 1) );
            offset += 4;
        }
        else
        {
            item->zone = (const char *)offset;
            offset += strlen(offset) + 1;
        }

        if( 0x02 == *offset )
            item->area = (const char *)( ptr + get_3b(offset + 1) );
        else
            item->area = (const char *)offset;
        return true;
    }
    return false;
}

static bool qqwry_find(const ipdb *ctx, ipdb_item *item, uint32_t ip)
{
    char *ptr = (char*)ctx->buffer;
    char *p = ptr + *(uint32_t*)ptr;

    uint32_t low = 0;
    uint32_t high = ctx->count;
    while (1)
    {
        if( low >= high - 1 )
            break;

        if( ip < *(uint32_t*)(p + (low + high)/2 * 7) )
            high = (low + high)/2;
        else
            low = (low + high)/2;
    }
    return qqwry_iter(ctx, item, low);
}

ipdb* qqwry_create(const uint8_t *buffer, uint32_t length)
{
    ipdb* ctx = ipdb_create();
    ctx->buffer = buffer;
    ctx->length = length;
    ctx->iter = qqwry_iter;
    ctx->find = qqwry_find;

    if(length>=8)
    {
        uint32_t *pos = (uint32_t*)buffer;
        uint32_t idx_first = *pos;
        uint32_t idx_last = *(pos + 1);
        ctx->count = idx_last - idx_first;
        if( (ctx->count % 7 == 0) && (length - idx_last == 7) )
        {
            ctx->count /= 7;
            ctx->count++;

            ipdb_item item;
            qqwry_iter(ctx, &item, ctx->count-1);
            uint32_t year = 0, month = 0, day = 0;
            if( sscanf(item.area, "%d年%d月%d日", &year, &month, &day)!=3 ) // 纯真IP数据库
            {
                if( sscanf(item.area, "%4d%2d%2d", &year, &month, &day)!=3 ) // 珊瑚虫IP数据库
                {
                    year = 1899, month = 12, day = 30; // 未知数据库
                }
            }
            ctx->date = year*10000 + month*100 + day;
        }
        else
        {
            ctx->count = 0;
        }
    }
    return ctx;
}
