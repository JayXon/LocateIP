#include "ipdb.h"
#include "util.h"

ipdb* ipdb_create(const ipdb_handle *handle, const uint8_t *buffer, uint32_t length)
{
    ipdb* ctx = calloc(1, sizeof(ipdb));
    ctx->handle = handle;
    ctx->handle->init(ctx, buffer, length);
    return ctx;
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
    return ctx->handle->find(ctx, item, str2ip(ip));
}

bool ipdb_next(ipdb_iter *iter, ipdb_item *item)
{
    return iter->ctx->handle->iter(iter->ctx, item, iter->index++);
}
