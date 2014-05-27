#include <stdio.h>
#include "ipdb.c"
#include "util.c"
#include "qqwry.c"
#include "mon17.c"

//#include <windows.h>
uint8_t* readfile(const char *path, uint32_t *length)
{
    FILE *fp = fopen(path, "rb");
    if(fp)
    {
        fseek(fp, 0, SEEK_END);
        *length = ftell(fp);
        uint8_t* buffer = (uint8_t*)malloc(*length);

        fseek(fp, 0, SEEK_SET);
        fread(buffer, *length, 1, fp);
        fclose(fp);
        return buffer;
    }
    return 0;
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
    uint8_t *buffer = readfile("qqwry.dat", &length);
    ipdb *db = ipdb_create(&qqwry_handle, buffer, length);



    if(buffer) free(buffer);
    ipdb_release(db);
}
int main()
{
    test_read_qqwry();
    test_read_mon17();
    //getchar();
    return 0;
}
