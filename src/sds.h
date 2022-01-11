/* SDSLib, A C dynamic strings library
 *
 * Copyright (c) 2006-2010, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __SDS_H
#define __SDS_H

#define SDS_MAX_PREALLOC (1024*1024)            // 备用空间大小

#include <sys/types.h>
#include <stdarg.h>

typedef char *sds;

/* 简单动态字符串，二进制安全的
 * 实现string，AOF缓冲区，客户端状态中的输入缓存区
 * */
struct sdshdr {
    unsigned int len;		// buff中字符串的长度，不含'\0'
    unsigned int free;		// buff中未使用的空间
    char buf[];				// 字节数组，数组中间可以出现'\0'，并以'\0'结尾以兼容c函数
};

// 获取sds的长度
static inline size_t sdslen(const sds s) {
    struct sdshdr *sh = (void*)(s-(sizeof(struct sdshdr)));
    return sh->len;
}

// 获取sds的空闲空间
static inline size_t sdsavail(const sds s) {
    struct sdshdr *sh = (void*)(s-(sizeof(struct sdshdr)));
    return sh->free;
}

// 创建sds
// 分配initlen长度的空间，将init中数据复制进去，没有备用空间
sds sdsnewlen(const void *init, size_t initlen);
// 以init的大小创建sds
sds sdsnew(const char *init);
// 创建长度为0的sds
sds sdsempty(void);

// sds已用空间长度
size_t sdslen(const sds s);
// 复制sds
sds sdsdup(const sds s);
// 释放sds
void sdsfree(sds s);
// sds备用空间长度
size_t sdsavail(const sds s);

// 扩大空间，传入的sds和返回sds可能在不同的地址上
// 将s的已用空间扩大，如果备用空间够大，则不处理
sds sdsgrowzero(sds s, size_t len);

// 追加字符串
// 追加长度为len的t到sds中
sds sdscatlen(sds s, const void *t, size_t len);
// 追加t到sds中，t必须以'\0'结尾
sds sdscat(sds s, const char *t);
// 等价于sdscat，t可以不以'\0'结尾
sds sdscatsds(sds s, const sds t);

// 将长度为len的t覆盖s
sds sdscpylen(sds s, const char *t, size_t len);
// 将t覆盖s，t将必须以'\0'结尾
sds sdscpy(sds s, const char *t);

sds sdscatvprintf(sds s, const char *fmt, va_list ap);
#ifdef __GNUC__
sds sdscatprintf(sds s, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));
#else
sds sdscatprintf(sds s, const char *fmt, ...);
#endif

sds sdscatfmt(sds s, char const *fmt, ...);
// 从左右两边删除s中含有的cset中的字符
sds sdstrim(sds s, const char *cset);
// 返回s中[start,end]范围的字符串，如果start>end则清空，支持正负索引
void sdsrange(sds s, int start, int end);
void sdsupdatelen(sds s);
void sdsclear(sds s);

int sdscmp(const sds s1, const sds s2);
sds *sdssplitlen(const char *s, int len, const char *sep, int seplen, int *count);
void sdsfreesplitres(sds *tokens, int count);
void sdstolower(sds s);
void sdstoupper(sds s);
sds sdsfromlonglong(long long value);
sds sdscatrepr(sds s, const char *p, size_t len);
sds *sdssplitargs(const char *line, int *argc);
sds sdsmapchars(sds s, const char *from, const char *to, size_t setlen);
sds sdsjoin(char **argv, int argc, char *sep);

/* Low level functions exposed to the user API */
sds sdsMakeRoomFor(sds s, size_t addlen);
void sdsIncrLen(sds s, int incr);
sds sdsRemoveFreeSpace(sds s);
size_t sdsAllocSize(sds s);

#endif
