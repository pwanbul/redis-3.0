/* endinconv.c -- Endian conversions utilities.
 *
 * 这个函数永远不会被直接调用，而是总是使用定义在 endianconv.h 中的宏，
 * 这样我们定义的一切都是非操作的，如果 arch 已经是小端。
 *
 * Redis 尝试将所有内容都编码为小端（但一些需要向后兼容的东西仍然是大端）
 * 因为大多数生产环境都是小端，我们在一些地方进行了大量转换，因为 ziplists、intsets , zipmaps，
 * 即使在内存中也需要是字节序中性的，因为它们直接在RDB文件上使用单个write(2)进行序列化，无需其他额外步骤。
 *
 * ----------------------------------------------------------------------------
 *
 * Copyright (c) 2011-2012, Salvatore Sanfilippo <antirez at gmail dot com>
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

/* 将p指向的16位无符号整数从小端切换到大端
 * 0 1 -> 1 0
 * */
void memrev16(void *p) {
    unsigned char *x = p, t;

    t = x[0];
    x[0] = x[1];
    x[1] = t;
}

/* 将p指向的32位无符号整数从小端切换到大端
 * 0 1 2 3 -> 3 2 1 0
 * */
void memrev32(void *p) {
    unsigned char *x = p, t;

    t = x[0];
    x[0] = x[3];
    x[3] = t;
    t = x[1];
    x[1] = x[2];
    x[2] = t;
}

/* 将p指向的64位无符号整数从小端切换到大端
 * 0 1 2 3 4 5 6 7 -> 7 6 5 4 3 2 1 0
 * */
void memrev64(void *p) {
    unsigned char *x = p, t;

    t = x[0];
    x[0] = x[7];
    x[7] = t;
    t = x[1];
    x[1] = x[6];
    x[6] = t;
    t = x[2];
    x[2] = x[5];
    x[5] = t;
    t = x[3];
    x[3] = x[4];
    x[4] = t;
}

uint16_t intrev16(uint16_t v) {
    memrev16(&v);
    return v;
}

uint32_t intrev32(uint32_t v) {
    memrev32(&v);
    return v;
}

uint64_t intrev64(uint64_t v) {
    memrev64(&v);
    return v;
}

#ifdef TESTMAIN
#include <stdio.h>

int main(void) {
    char buf[32];

    sprintf(buf,"ciaoroma");
    memrev16(buf);
    printf("%s\n", buf);

    sprintf(buf,"ciaoroma");
    memrev32(buf);
    printf("%s\n", buf);

    sprintf(buf,"ciaoroma");
    memrev64(buf);
    printf("%s\n", buf);

    return 0;
}
#endif
