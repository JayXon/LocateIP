#include "ipdb.h"

bool qqwry_build(const ipdb *, const char *);

// memory buffer
typedef struct
{
    uint8_t*        data;
    uint32_t        size;
    uint32_t        capacity;
} buffer;

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
    free(buf->data);
    free(buf);
}

// hash table
typedef struct table_node_t table_node;
struct table_node_t
{
    uint32_t        key;
    table_node*     next;
    uint32_t        value;
};

typedef struct
{
    uint32_t        hash;
    char            str[0];
} table_key;

typedef struct
{
    table_node*     head;
    table_node*     idle;
    uint32_t        size;
    uint32_t        seed;
    buffer*         str;
} table;

// assist
uint32_t calc_hash(const char* str, size_t len, uint32_t seed)
{
    uint32_t h = (uint32_t)len ^ seed;
    size_t step = (len >> 5) + 1;
    size_t i = len;
    for (; i >= step; i -= step)
        h = h ^ ((h << 5) + (h >> 2) + (uint32_t)str[i - 1]);
    return h & (0xFFFFFFFF >> 10);
}
const table_key* make_table_key(const char* str, size_t len, uint32_t seed)
{
    static char buffer[1 << 10];
    table_key* key = (table_key*)buffer;
    key->hash = (len << 22) | calc_hash(str, len, seed);
    memcpy(key->str, str, len * sizeof(char));
    return key;
}
uint32_t get_len(const table_key* key)
{
    return key->hash >> 22;
}
const table_node* make_node(table *t, const table_key* key)
{
    static table_node node;
    node.key = buffer_size(t->str);
    buffer_append(t->str, key, sizeof(table_key));
    buffer_append(t->str, key->str, get_len(key));
    node.next = 0;
    node.value = 0;
    return &node;
}
uint32_t get_position(table *t, const table_key* key)
{
    return key->hash & (t->size - 1);
}
bool is_empty_node(const table_node* node)
{
    return node->key == 0;
}
bool is_same_key(const table_key* key, const table_key* cmpkey)
{
    return key->hash == cmpkey->hash && memcmp(key->str, cmpkey->str, get_len(cmpkey)) == 0;
}
const table_key* get_key(table *t, const table_node* node)
{
    uint8_t* buf = buffer_get(t->str);
    return (const table_key*)&buf[node->key];
}
table_node* get_next(const table_node* node)
{
    return node->next;
}
table_node* get_idle_node(table *t)
{
    while (t->idle > t->head)
    {
        t->idle--;
        if (is_empty_node(t->idle)) return t->idle;
    }
    return 0;
}
void set_node(table_node* node, const table_node* data)
{
    memcpy(node, data, sizeof(table_node));
}
void resize(table *t, uint32_t newsize)
{
    uint32_t i = 0;
    uint32_t oldsize = t->size;
    table_node* oldhead = t->head;
    t->size = newsize;
    t->head = (table_node*)calloc(t->size, sizeof(table_node));
    t->idle = t->head + t->size;
    for (; i < oldsize; i++)
    {
        table_node* node = oldhead + i;
        if (!is_empty_node(node))
        {
            node->next = 0;

            table_node* table_insert(table *t, const table_node* data);
            table_insert(t, node);
        }
    }
    free(oldhead);
}
table_node* table_find(table *t, const table_key* key)
{
    table_node* node = t->head + get_position(t, key);
    while (!is_empty_node(node))
    {
        if (is_same_key(get_key(t, node), key)) return node;
        node = get_next(node);
        if (!node) break;
    }
    return 0;
}
table_node* table_insert(table *t, const table_node* data)
{
    uint32_t position = get_position(t, get_key(t, data));
    table_node* node = t->head + position;
    if (!is_empty_node(node))
    {
        if (!is_same_key(get_key(t, node), get_key(t, data)))
        {
            table_node* next = get_idle_node(t);
            if (next == NULL)
            {
                resize(t, t->size << 1);
                return table_insert(t, data);
            }
            else
            {
                table_node* other = t->head + get_position(t, get_key(t, node));
                if (other != node)
                {
                    while (get_next(other) != node) other = get_next(other);
                    other->next = next;
                    set_node(next, node);
                }
                else
                {
                    while (get_next(other)) other = get_next(other);
                    other->next = next;
                    set_node(next, data);
                    return next;
                }
            }
        }
    }
    set_node(node, data);
    return node;
}
//
table *table_create()
{
    table *t = calloc(1, sizeof(table));
    t->size = 1;
    t->head = calloc(t->size, sizeof(table_node));
    t->idle = t->head + t->size;
    t->seed = (uint32_t)&t->head;
    t->str = buffer_create();
    buffer_append(t->str, &t->head, sizeof(uint32_t)); // hash seed
    return t;
}

table_node* table_set_key(table *t, const char* name)
{
    const table_key *key = make_table_key(name, strlen(name), t->seed);
    table_node *node = table_find(t, key);
    if(!node) return table_insert(t, make_node(t, key));
    return node;
}

table_node* table_get_key(table *t, const char* name)
{
    const table_key *key = make_table_key(name, strlen(name), t->seed);
    return table_find(t, key);
}

void table_release(table *t)
{
    buffer_release(t->str);
    free(t->head);
    free(t);
}

void show_table_key(table *t)
{
    uint32_t i = 0;
    for(;i<t->size;i++)
    {
        table_node* node = t->head + i;
        if (!is_empty_node(node))
        {
            const table_key* key = get_key(t, node);
            printf("%.*s\n", get_len(key), key->str);
        }
    }
}

//
typedef struct
{
    uint32_t        offset;
    table*          extend;
} table_value;

table_value* make_table_value(table_node *node, uint32_t offset)
{
    table_value *node_value = calloc(1, sizeof(table_value));
    node_value->offset = offset;
    node_value->extend = table_create();
    node->value = (uint32_t)node_value;
    return node_value;
}
void release_table_value(table *t)
{
    uint32_t i = 0;
    for(;i<t->size;i++)
    {
        table_node* node = t->head + i;
        if (!is_empty_node(node))
        {
            table_value *node_value = (table_value*)node->value;
            release_table_value(node_value->extend);
            table_release(node_value->extend);
            free(node_value);
        }
    }
}

bool qqwry_build(const ipdb *ctx, const char *file)
{
    buffer *record_buffer = buffer_create();
    buffer *index_buffer = buffer_create();
    table *string_table = table_create();

    uint32_t offset = 8;

    ipdb_iter iter = {ctx, 0};
    ipdb_item item;
    while(ipdb_next(&iter, &item))
    {
        buffer_append(record_buffer, &item.upper, sizeof(item.upper));

        offset += 4;

        table_node *zone = table_get_key(string_table, item.zone);
        if(zone)
        {
            table_value *zone_value = (table_value*)zone->value;
            table_node *area = table_get_key(zone_value->extend, item.area);
            if(area)
            {
                table_value *area_value = (table_value*)area->value;
                uint8_t redirect = 0x01;
                buffer_append(record_buffer, &redirect, sizeof(redirect));
                buffer_append(record_buffer, &area_value->offset, 3);
            }
            else
            {
                table_node *node = table_set_key(zone_value->extend, item.area);
                make_table_value(node, offset);

                uint8_t redirect = 0x02;
                buffer_append(record_buffer, &redirect, sizeof(redirect));
                buffer_append(record_buffer, &zone_value->offset, 3);

                area = table_get_key(string_table, item.area);
                uint32_t area_len = strlen(item.area) + 1;
                if(area_len>4 && area )
                {
                    table_value *area_value = (table_value*)area->value;
                    buffer_append(record_buffer, &redirect, sizeof(redirect));
                    buffer_append(record_buffer, &area_value->offset, 3);
                }
                else
                {
                    area = table_set_key(string_table, item.area);
                    make_table_value(area, offset + 4);

                    buffer_append(record_buffer, item.area, area_len);
                }
            }
        }
        else
        {
            zone = table_set_key(string_table, item.zone);
            table_value *zone_value = make_table_value(zone, offset);

            uint32_t zone_len = strlen(item.zone) + 1;
            buffer_append(record_buffer, item.zone, zone_len);

            table_node *node = table_set_key(zone_value->extend, item.area);
            table_value *node_value = calloc(1, sizeof(table_value));
            node->value = (uint32_t)node_value;
            node_value->offset = offset;
            node_value->extend = table_create();

            table_node *area = table_get_key(string_table, item.area);

            uint32_t area_len = strlen(item.area) + 1;
            if(area_len>4 && area)
            {
                table_value *area_value = (table_value*)area->value;

                uint8_t redirect = 0x02;
                buffer_append(record_buffer, &redirect, sizeof(redirect));
                buffer_append(record_buffer, &area_value->offset, 3);
            }
            else
            {
                area = table_set_key(string_table, item.area);
                make_table_value(area, offset + zone_len);

                buffer_append(record_buffer, item.area, area_len);
            }
        }

        offset -= 4;
        buffer_append(index_buffer, &item.lower, sizeof(item.lower));
        buffer_append(index_buffer, &offset, 3);

        offset = buffer_size(record_buffer) + 8;
    }

    uint32_t idx_first = offset;
    uint32_t idx_last = offset + buffer_size(index_buffer) - 7;

    FILE * fp = fopen(file, "wb");
    if(fp)
    {
        fwrite(&idx_first, 1, sizeof(idx_first), fp);
        fwrite(&idx_last, 1, sizeof(idx_last), fp);

        fwrite(buffer_get(record_buffer), 1, buffer_size(record_buffer), fp);
        fwrite(buffer_get(index_buffer), 1, buffer_size(index_buffer), fp);
        fclose(fp);
    }

    release_table_value(string_table);
    table_release(string_table);
    buffer_release(index_buffer);
    buffer_release(record_buffer);
    return true;
}

void test_table()
{
    table *test = table_create();

    table_set_key(test, "联通");
    table_set_key(test, "电信");
    table_set_key(test, "电信");
    table_set_key(test, "安徽职业技术学院");

    show_table_key(test);
    table_release(test);
}
