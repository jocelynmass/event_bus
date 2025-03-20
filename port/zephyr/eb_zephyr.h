/*
 * MIT License
 *
 * Copyright (c) 2019 Jocelyn Masserot
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal with the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 *  1. The above copyright notice and this permission notice shall be included in all
 *     copies or substantial portions of the Software.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimers in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Jocelyn Masserot, nor the names of its contributors
 *     may be used to endorse or promote products derived from this Software
 *     without specific prior written permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * WITH THE SOFTWARE.
 */

#ifndef __EB_ZEPHYR_PORT_H__
#define __EB_ZEPHYR_PORT_H__

#include <zephyr/kernel.h>

#define K_HEAP_EXPAND(idx, array, k_heap_len) \
    K_HEAP_DEFINE(array##_##idx, k_heap_len);

#define K_HEAP_PTR_EXPAND(idx, array) \
    &array##_##idx,

#define K_HEAP_ARRAY_DEFINE(array, nb_k_heap, k_heap_len) \
    LISTIFY(nb_k_heap, K_HEAP_EXPAND, (), array, k_heap_len) \
    struct k_heap *array[nb_k_heap] = { LISTIFY(nb_k_heap, K_HEAP_PTR_EXPAND, (), array) };


typedef struct z_th_t
{
    uint8_t idx;
    k_tid_t tid;
    struct k_heap *heap; 
}z_th_t;

typedef struct k_msgq eb_queue_t;
typedef struct k_mutex eb_mutex_t;
typedef z_th_t* eb_thread_t;

typedef void (eb_thread_func)(void *arg1, void *arg2, void *arg3);

#endif //__EB_ZEPHYR_PORT_H__