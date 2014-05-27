#include "ipdb.h"

// http://tool.17mon.cn/ipdb.html

// 查询更新 http://api.17mon.cn/api.php?a=ipdb
// 返回字符串 112.121.182.84|20140501|http://s.qdcdn.com/17mon/17monipdb.dat

inline static uint32_t swap32(uint32_t n)
{
    #ifdef __GNUC__
        return __builtin_bswap32(n);
    #elif defined _MSC_VER
        return _byteswap_ulong(n);
    #else
        //
    #endif // __GNUC__
}

typedef struct
{
    uint32_t upper;
    uint32_t offset:24;
    uint32_t length:8;
} mon17_item;

static bool mon17_iter(const ipdb *ctx, ipdb_item *item, uint32_t index)
{
    static char buf[256];
    if(index<ctx->count)
    {
        mon17_item *ptr = (mon17_item*)(ctx->buffer + 4 + 256*4);

        const char *text = (const char*)ctx->buffer + 4 + 256*4 + ctx->count*8 + ptr[index].offset;

        item->lower = index==0?0:(swap32(ptr[index-1].upper)+1);
        item->upper = swap32(ptr[index].upper);

        memcpy(buf, text, ptr[index].length);
        buf[ptr[index].length] = 0;

        char *parting = strchr(buf, '\t');
        *parting = 0;

        item->zone = buf;
        item->area = parting + 1;
        return true;
    }
    return false;
}

static bool mon17_find(const ipdb *ctx, ipdb_item *item, uint32_t ip)
{
    uint32_t *index = (uint32_t*)(ctx->buffer + 4);
    uint32_t offset = index[ip>>24];
    uint32_t _ip = swap32(ip);

    mon17_item *ptr = (mon17_item*)(ctx->buffer + 4 + 256*4);
    for(;offset<ctx->count;offset++)
    {
        if( memcmp(&ptr[offset].upper, &_ip, 4)>=0 )
        {
            break;
        }
    }
    return mon17_iter(ctx, item, offset);
}

static bool mon17_init(ipdb* ctx, const uint8_t *buffer, uint32_t length)
{
    ctx->buffer = buffer;
    ctx->length = length;

    if(length>=4 && sizeof(mon17_item)==8)
    {
        uint32_t *pos = (uint32_t*)buffer;
        uint32_t index_length = swap32(*pos);
        ctx->count = (index_length - 4 - 256*4 - 1024)/8;

        ipdb_item item;
        mon17_iter(ctx, &item, ctx->count-1);
        uint32_t year = 0, month = 0, day = 0;
        if( sscanf(item.area, "%4d%2d%2d", &year, &month, &day)!=3 ) // 17mon数据库
        {
            year = 1899, month = 12, day = 30; // 未知数据库
        }
        ctx->date = year*10000 + month*100 + day;
    }
    return ctx->count!=0;
}

const ipdb_handle mon17_handle = {mon17_init, mon17_iter, mon17_find};
