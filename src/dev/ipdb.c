#include "ipdb.h"

static char* ip2str(char *buf, size_t len, int ip)
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

static uint32_t str2ip(const char *lp)
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

ipdb* ipdb_create()
{
    return calloc(1, sizeof(ipdb));
}

void ipdb_release(ipdb *ctx)
{
    free(ctx);
}

bool ipdb_dump(const ipdb *ctx, const char *file)
{
    FILE *fp = fopen(file, "wb");
    if(fp)
    {
        ipdb_iter iter;
        iter.ctx = ctx;
        iter.index = 0;
        ipdb_item item;
        while(ipdb_next(&iter, &item))
        {
            char ip1[16];
            char ip2[16];

            char *ip1_t = ip2str(ip1, sizeof(ip1), item.lower);
            char *ip2_t = ip2str(ip2, sizeof(ip2), item.upper);
            fprintf(fp, "%-16s%-16s%s%s%s\r\n", ip1_t, ip2_t, item.zone, strlen(item.area)>0?" ":"", item.area);
        }

        //fprintf(fp, "\r\n\r\nIP数据库共有数据 ： %d 条\r\n", ctx->count);
        fclose(fp);
        return true;
    }
    return false;
}

bool ipdb_find(const ipdb *ctx, ipdb_item *item, const char *ip)
{
    return ctx->find(ctx, item, str2ip(ip));
}

bool ipdb_next(ipdb_iter *iter, ipdb_item *item)
{
    return iter->ctx->iter(iter->ctx, item, iter->index++);
}
