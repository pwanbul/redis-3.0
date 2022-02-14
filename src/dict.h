/* Hash Tables Implementation.
 *
 * This file implements in-memory hash tables with insert/del/replace/find/
 * get-random-element operations. Hash tables will auto-resize if needed
 * tables of power of two in size are used, collisions are handled by
 * chaining. See the source code for more information... :)
 *
 * Copyright (c) 2006-2012, Salvatore Sanfilippo <antirez at gmail dot com>
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

#include <stdint.h>

#ifndef __DICT_H
#define __DICT_H

#define DICT_OK 0
#define DICT_ERR 1

/* 未使用的参数会产生烦人的警告... */
#define DICT_NOTUSED(V) ((void) V)

// 用于实现dict，数据库(struct redisDB)，即5种键值对数据类型
// hash节点
typedef struct dictEntry {
    void *key;          // 键
    union {
        void *val;
        uint64_t u64;
        int64_t s64;            // 保存超时时间戳，毫秒
        double d;
    } v;            // 值
    struct dictEntry *next;         // 下一个节点，单链表，头插
} dictEntry;

typedef struct dictType {
    unsigned int (*hashFunction)(const void *key);              // hash函数
    void *(*keyDup)(void *privdata, const void *key);           // 复制key
    void *(*valDup)(void *privdata, const void *obj);           // 复制value
    int (*keyCompare)(void *privdata, const void *key1, const void *key2);      // 比较key
    void (*keyDestructor)(void *privdata, void *key);       // 释放key
    void (*valDestructor)(void *privdata, void *obj);       // 释放value
} dictType;

/* 这是我们的哈希表结构。 每个字典都有两个这样的，因为我们实现了增量重新散列，从旧表到新表。 */
typedef struct dictht {
    dictEntry **table;          // hash桶，链表中的节点由dictEntry实现
    unsigned long size;         // hash桶的大小
    unsigned long sizemask;     // hash桶的大小掩码
    unsigned long used;         // hash表中已有的节点数量
} dictht;

// 字典
typedef struct dict {
    dictType *type;         // 类型特定函数，指向struct dictType
    void *privdata;         // 传递给struct dictType中函数的参数
    dictht ht[2];           // hash桶，一般只使用ht[0]，ht[1]用于对ht[0]rehash
    long rehashidx; /* 记录rehash的进度，如果rehashidx==-1，则不会进行重新哈希 */
    int iterators; /* 当前运行的迭代器数量 */
} dict;

/* 如果 safe 设置为 1，则这是一个安全迭代器，这意味着，即使在迭代过程中，您也可以针对字典调用 dictAdd、dictFind 和其他函数。
 * 否则它是一个不安全的迭代器，迭代时只应调用dictNext()。
 * */
typedef struct dictIterator {
    dict *d;
    long index;
    int table, safe;
    dictEntry *entry, *nextEntry;
    /* 用于误用检测的不安全迭代器指纹。 */
    long long fingerprint;
} dictIterator;

typedef void (dictScanFunction)(void *privdata, const dictEntry *de);

/* This is the initial size of every hash table */
#define DICT_HT_INITIAL_SIZE     4

/* ------------------------------- Macros ------------------------------------*/
#define dictFreeVal(d, entry) \
    if ((d)->type->valDestructor) \
        (d)->type->valDestructor((d)->privdata, (entry)->v.val)

// 设置value
#define dictSetVal(d, entry, _val_) do { \
    if ((d)->type->valDup) \
        entry->v.val = (d)->type->valDup((d)->privdata, _val_); \
    else \
        entry->v.val = (_val_); \
} while(0)

#define dictSetSignedIntegerVal(entry, _val_) \
    do { entry->v.s64 = _val_; } while(0)

#define dictSetUnsignedIntegerVal(entry, _val_) \
    do { entry->v.u64 = _val_; } while(0)

#define dictSetDoubleVal(entry, _val_) \
    do { entry->v.d = _val_; } while(0)

#define dictFreeKey(d, entry) \
    if ((d)->type->keyDestructor) \
        (d)->type->keyDestructor((d)->privdata, (entry)->key)

// 设置key
#define dictSetKey(d, entry, _key_) do { \
    if ((d)->type->keyDup) \
        entry->key = (d)->type->keyDup((d)->privdata, _key_); \
    else \
        entry->key = (_key_); \
} while(0)

#define dictCompareKeys(d, key1, key2) \
    (((d)->type->keyCompare) ? \
        (d)->type->keyCompare((d)->privdata, key1, key2) : \
        (key1) == (key2))

// 计算hash值
#define dictHashKey(d, key) (d)->type->hashFunction(key)
#define dictGetKey(he) ((he)->key)
#define dictGetVal(he) ((he)->v.val)
#define dictGetSignedIntegerVal(he) ((he)->v.s64)
#define dictGetUnsignedIntegerVal(he) ((he)->v.u64)
#define dictGetDoubleVal(he) ((he)->v.d)
#define dictSlots(d) ((d)->ht[0].size+(d)->ht[1].size)
#define dictSize(d) ((d)->ht[0].used+(d)->ht[1].used)
/* 是否有rehash进度
 * 如果有进度
 *   1. 不能调整hash表的大小
 *   2. 不能再次触发rehash
 *   3. 插入/删除/查询/遍历/随机等操作会触发ht[0]向ht[1]调整数据
 *   4. 新加入的key，会放入ht[1]中
 * */
#define dictIsRehashing(d) ((d)->rehashidx != -1)

/* 函数原型 */
// 创建一个新的哈希表
dict *dictCreate(dictType *type, void *privDataPtr);
// 扩大hash表
int dictExpand(dict *d, unsigned long size);
// 插入键值对
int dictAdd(dict *d, void *key, void *val);
// 插入key
dictEntry *dictAddRaw(dict *d, void *key);
// 替换键值对，key不存在则加入
int dictReplace(dict *d, void *key, void *val);
// 替换key，key不存在则加入
dictEntry *dictReplaceRaw(dict *d, void *key);
// 删除键值对
int dictDelete(dict *d, const void *key);
// 删除键值对，不删除privdata中的数据
int dictDeleteNoFree(dict *d, const void *key);
// 删除整个dict
void dictRelease(dict *d);
// 按key查找dictEntry
dictEntry * dictFind(dict *d, const void *key);
// 按key查找value
void *dictFetchValue(dict *d, const void *key);
// 使负载因子接近1
int dictResize(dict *d);

dictIterator *dictGetIterator(dict *d);
dictIterator *dictGetSafeIterator(dict *d);
dictEntry *dictNext(dictIterator *iter);
void dictReleaseIterator(dictIterator *iter);
dictEntry *dictGetRandomKey(dict *d);
unsigned int dictGetSomeKeys(dict *d, dictEntry **des, unsigned int count);
void dictPrintStats(dict *d);
unsigned int dictGenHashFunction(const void *key, int len);
unsigned int dictGenCaseHashFunction(const unsigned char *buf, int len);
void dictEmpty(dict *d, void(callback)(void*));
// 使能rehash
void dictEnableResize(void);
// 禁用rehash
void dictDisableResize(void);
int dictRehash(dict *d, int n);
int dictRehashMilliseconds(dict *d, int ms);
void dictSetHashFunctionSeed(unsigned int initval);
unsigned int dictGetHashFunctionSeed(void);
unsigned long dictScan(dict *d, unsigned long v, dictScanFunction *fn, void *privdata);

/* Hash table types */
extern dictType dictTypeHeapStringCopyKey;
extern dictType dictTypeHeapStrings;
extern dictType dictTypeHeapStringCopyKeyValue;

#endif /* __DICT_H */
