#include <stdio.h>
#include <stdlib.h>
int calloc_times = 0;
int free_times = 0;
void* my_malloc(size_t len)
{
    calloc_times++;
    return malloc(len);
}
void* my_calloc(size_t n, size_t len)
{
    calloc_times++;
    return calloc(n, len);
}
void* my_realloc(void *ptr, size_t len)
{
    if(ptr==0) calloc_times++;
    return realloc(ptr, len);
}
void my_free(void *ptr)
{
    free_times++;
    free(ptr);
}
#define malloc my_malloc
#define calloc my_calloc
#define realloc my_realloc
#define free my_free
#include "ipdb.c"
#include "util.c"
#include "qqwry.c"
#include "mon17.c"
#include "qqwry_build.c"
#include "patch.c"

//#include <windows.h>
uint8_t* readfile(const char *path, uint32_t *length)
{
    uint8_t* buffer = 0;
    FILE *fp = fopen(path, "rb");
    if(fp)
    {
        fseek(fp, 0, SEEK_END);
        *length = ftell(fp);
        buffer = (uint8_t*)malloc(*length);

        fseek(fp, 0, SEEK_SET);
        fread(buffer, *length, 1, fp);
        fclose(fp);
    }
    return buffer;
}

void test_read_qqwry()
{
    uint32_t length = 0;
    uint8_t *buffer = readfile("qqwry.dat", &length);
    ipdb *db = ipdb_create(&qqwry_handle, buffer, length);
    printf("%d %d\n", db->count, db->date);

    if(db->count)
    {
        //ipdb_dump(db, "qqwry.txt");

        ipdb_item item;
        if( ipdb_find(db, &item, "112.121.182.84") )
        {
            char ip1[16];
            char ip2[16];

            char *ip1_t = ip2str(ip1, sizeof(ip1), item.lower);
            char *ip2_t = ip2str(ip2, sizeof(ip2), item.upper);
            printf("%s %s %s %s\n", ip1_t, ip2_t, item.zone, item.area);
        }
    }

    if(buffer) free(buffer);
    ipdb_release(db);
}

void test_read_mon17()
{
    uint32_t length = 0;
    uint8_t *buffer = readfile("17monipdb.dat", &length);
    ipdb *db = ipdb_create(&mon17_handle, buffer, length);
    printf("%d %d\n", db->count, db->date);

    if(db->count)
    {
        //ipdb_dump(db, "17mon.txt");

        ipdb_item item;
        if( ipdb_find(db, &item, "112.121.182.84") )
        {
            char ip1[16];
            char ip2[16];

            char *ip1_t = ip2str(ip1, sizeof(ip1), item.lower);
            char *ip2_t = ip2str(ip2, sizeof(ip2), item.upper);
            printf("%s %s %s %s\n", ip1_t, ip2_t, item.zone, item.area);
        }
    }

    if(buffer) free(buffer);
    ipdb_release(db);
}

void test_build_qqwry()
{
    uint32_t length = 0;
    uint8_t *buffer = readfile("qqwry_old.dat", &length);
    ipdb *db = ipdb_create(&qqwry_handle, buffer, length);

    if(db->count) qqwry_build(db, "qqwry.dat");

    if(buffer) free(buffer);
    ipdb_release(db);

}
void test_build_patch()
{
    uint32_t length1 = 0;
    uint32_t length2 = 0;
    uint8_t *buffer1 = readfile("qqwry 325.dat", &length1);
    uint8_t *buffer2 = readfile("qqwry 525.dat", &length2);
    ipdb *db1 = ipdb_create(&qqwry_handle, buffer1, length1);
    ipdb *db2 = ipdb_create(&qqwry_handle, buffer2, length2);

    if(db1->count&&db2->count) make_patch(db1, db2);

    if(buffer1) free(buffer1);
    if(buffer2) free(buffer2);
    ipdb_release(db1);
    ipdb_release(db2);
}
int main()
{
    //test_table();
    //test_read_qqwry();
    //test_read_mon17();
    //test_build_qqwry();
    //test_build_patch();
    printf("calloc_times:%d free_times:%d %d\n",calloc_times,free_times,calloc_times-free_times);
    return 0;
}
