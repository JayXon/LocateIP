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
