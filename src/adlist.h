/* adlist.h - A generic doubly linked list implementation
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

#ifndef __ADLIST_H__
#define __ADLIST_H__

/* Node、List 和 Iterator 是目前唯一使用的数据结构。
 *
 * 双向无环链表，用于实现list，发布和订阅，慢查询，监视器
 * */

// 链表节点
typedef struct listNode {
    struct listNode *prev;      // 前一个链表节点
    struct listNode *next;      // 后一个链表节点
    void *value;        // 用户数据
} listNode;

// 迭代器
typedef struct listIter {
    listNode *next;     // 右边/左边的节点
    int direction;      // 方向
} listIter;

// 链表头
typedef struct list {
    listNode *head;         // 指向链表中第一个节点
    listNode *tail;         // 指向链表中最后一个节点
    void *(*dup)(void *ptr);        // 复制节点callback
    void (*free)(void *ptr);        // 释放节点callback
    int (*match)(void *ptr, void *key);     // 节点比较callback
    unsigned long len;      // 链表长度
} list;

/* 作为宏实现的函数 */
#define listLength(l) ((l)->len)        // 链表长度
#define listFirst(l) ((l)->head)        // 第一个节点
#define listLast(l) ((l)->tail)         // 最后一个节点
#define listPrevNode(n) ((n)->prev)     // 前一个节点
#define listNextNode(n) ((n)->next)     // 后一个节点
#define listNodeValue(n) ((n)->value)       // 用户数据地址

// 设置callback
#define listSetDupMethod(l,m) ((l)->dup = (m))
#define listSetFreeMethod(l,m) ((l)->free = (m))
#define listSetMatchMethod(l,m) ((l)->match = (m))

// 获取callback
#define listGetDupMethod(l) ((l)->dup)
#define listGetFree(l) ((l)->free)
#define listGetMatchMethod(l) ((l)->match)

/* 函数原型 */
// 创建管理器
list *listCreate(void);
// 释放整个链表，包括链表管理器，链表节点和链表节点中的数据
void listRelease(list *list);
// 头插链表
list *listAddNodeHead(list *list, void *value);
// 尾插链表
list *listAddNodeTail(list *list, void *value);
// 在old_node之前(之后)插入节点
list *listInsertNode(list *list, listNode *old_node, void *value, int after);
// 删除节点
void listDelNode(list *list, listNode *node);
// 返回正向/反向的迭代器
listIter *listGetIterator(list *list, int direction);
// 返回下一个节点，可以删除返回的节点，不会导致迭代器失效
listNode *listNext(listIter *iter);
// 释放迭代器
void listReleaseIterator(listIter *iter);
// 重点：复制整个链表，正向复制
list *listDup(list *orig);
// 在链表节点中搜索key，返回一个节点
listNode *listSearchKey(list *list, void *key);
// 返回index对应的节点，index可以为负数
listNode *listIndex(list *list, long index);
// 设置私有正向迭代器
void listRewind(list *list, listIter *li);
// 设置私有反向向迭代器
void listRewindTail(list *list, listIter *li);
// 把最后一个节点，插入链表头部
void listRotate(list *list);

/* 迭代器的方向 */
#define AL_START_HEAD 0         // 正向
#define AL_START_TAIL 1         // 反向

#endif /* __ADLIST_H__ */
