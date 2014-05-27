#ifndef __CZ_UPDATE_H_
#define __CZ_UPDATE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "ipdb.h"

// 本文件提供对纯真IP数据库自动更新的解析以及制作增量更新


// http://update.cz88.net/ip/copywrite.rar	元数据，280字节
// http://update.cz88.net/ip/qqwry.rar		数据库，zlib压缩，其中前0x200数据被加密

typedef struct
{
	uint32_t sign;		// "CZIP"文件头
	uint32_t version;	// 日期，用作版本号
	uint32_t unknown1;	// 未知数据，似乎永远取值0x01
	uint32_t size;		// qqwry.rar大小
	uint32_t unknown2;	// 未知数据
	uint32_t key;		// 解密qqwry.rar前0x200字节密钥
	char text[128];		// 提供商
	char link[128];		// 网址
} UpdateRecord, *UpdateContext;


UpdateContext ParseMetadata(const uint8_t*, uint32_t); // 解析更新数据，失败返回NULL

#ifdef __cplusplus
}
#endif

#endif // __CZ_UPDATE_H_
