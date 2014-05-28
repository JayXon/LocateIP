#include "ipdb.h"
#include "util.h"

bool make_patch(const ipdb *, const ipdb *);
bool apply_patch(const ipdb *, const uint8_t *, uint32_t, const char *);

//
typedef enum
{
    INSERT = 0,
    REMOVE = 1,
    MODIFY = 2
} patch_type;

typedef struct
{
    uint32_t            line:24;
    uint32_t            method:8;
    uint32_t            lower;
    uint32_t            zone;
    uint32_t            area;
} patch_item;

typedef struct
{
    uint32_t            magic;
    uint32_t            lower;
    uint32_t            zone;
    patch_item          item[0];
} patch_head;

bool make_patch(const ipdb *db1, const ipdb *db2)
{
    if(db1->date==db2->date) return false;
    if(db1->date>db2->date)
    {
        ipdb *temp = db1;
        db1 = db2;
        db2 = temp;
    }
    buffer *record_buffer = buffer_create();
    buffer *string_buffer = buffer_create();

    uint32_t i = 0;
    uint32_t j = 0;
    uint32_t k = 0;

    uint32_t n = 0;
    uint32_t m = 0;

    bool flag = false;

    ipdb_item item1;
    ipdb_item item2;

    while(i<db1->count && j<db2->count)
    {
        db1->handle->iter(db1, &item1, i);
        db2->handle->iter(db2, &item2, i);

        if(item1.lower!=item2.lower)
        {
            if(item1.lower<item2.lower) i++;
            else j++;

            flag = true;
            continue;
        }

        if(item1.upper!=item2.upper)
        {
            if(item1.upper<item2.upper) i++;
            else j++;

            flag = true;
            continue;
        }

        if(flag)
        {
            flag = false;

            for(k=0; k<i-n; k++)
            {
                patch_item item = {n + k, REMOVE, 0, 0, 0};
                buffer_append(record_buffer, &item, sizeof(patch_item));
            }

            for(k=0; k<j-m; k++)
            {
                db2->handle->iter(db2, &item2, m + k);
                uint32_t zone_offset = buffer_size(string_buffer);
                buffer_append(string_buffer, &item2.zone, strlen(item2.zone) + 1);
                uint32_t area_offset = buffer_size(string_buffer);
                buffer_append(string_buffer, &item2.area, strlen(item2.area) + 1);

                patch_item item = {i - 1, INSERT, item2.lower, zone_offset, area_offset};
                buffer_append(record_buffer, &item, sizeof(patch_item));
            }
        }

        db1->handle->iter(db1, &item1, i);
        db2->handle->iter(db2, &item2, i);
        if( strcmp(item1.zone, item2.zone) || strcmp(item1.area, item2.area) )
        {
            uint32_t zone_offset = buffer_size(string_buffer);
            buffer_append(string_buffer, &item2.zone, strlen(item2.zone) + 1);
            uint32_t area_offset = buffer_size(string_buffer);
            buffer_append(string_buffer, &item2.area, strlen(item2.area) + 1);

            patch_item item = {i, MODIFY, item2.lower, zone_offset, area_offset};
            buffer_append(record_buffer, &item, sizeof(patch_item));
        }

        i++;
        j++;
        n = i;
        m = j;
    }

    buffer_release(string_buffer);
    buffer_release(record_buffer);
    return true;
}

bool apply_patch(const ipdb *db, const uint8_t *buffer, uint32_t length, const char *file)
{

}
