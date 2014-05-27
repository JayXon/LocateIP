#ifndef __LOCATION_H_
#define __LOCATION_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

// 基本结构定义

#ifndef uint32_t
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned uint32_t;
#endif

typedef struct ipdb_t ipdb;
typedef struct ipdb_iter_t ipdb_iter;
typedef struct ipdb_item_t ipdb_item;

struct ipdb_t
{
	const uint8_t*		buffer;
	uint32_t            length;
	uint32_t			count;
	uint32_t			date;
	bool                (*iter)(const ipdb *, ipdb_item *, uint32_t);
	bool                (*find)(const ipdb *, ipdb_item *, uint32_t);
};

struct ipdb_iter_t
{
    const ipdb*         ctx;
    uint32_t            index;
};

struct ipdb_item_t
{
	const char*			zone;
	const char*			area;
	uint32_t			lower;
	uint32_t			upper;
};

ipdb* ipdb_create();
void ipdb_release(ipdb *);

bool ipdb_dump(const ipdb *, const char *);
bool ipdb_find(const ipdb *, ipdb_item *, const char *);
bool ipdb_next(ipdb_iter *, ipdb_item *);

#ifdef __cplusplus
}
#endif

#endif // __LOCATION_H_
