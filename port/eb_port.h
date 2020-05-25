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

#ifndef __EB_PORT_H__
#define __EB_PORT_H__

#ifdef USE_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"

typedef SemaphoreHandle_t eb_mutex_t;
typedef TaskHandle_t eb_thread_t;
typedef struct eb_timer_t
{
    TimerHandle_t hdl;
    void *arg;
    void (*cb)(void *arg);
}eb_timer_t;

#endif


int32_t eb_mutex_new(eb_mutex_t *mutex);
int32_t eb_mutex_take(eb_mutex_t *mutex, uint32_t timeout);
int32_t eb_mutex_give(eb_mutex_t *mutex);

eb_thread_t eb_thread_new(const char *name, void (*thread)(void *arg), void *arg, int stack_size, int prio);

uint32_t eb_get_tick(void);

int32_t eb_timer_new(eb_timer_t *timer, const char *name, uint32_t timeout_ms, void (*cb)(void *arg), void *arg);
int32_t eb_timer_start(eb_timer_t *timer);
int32_t eb_timer_stop(eb_timer_t *timer);
int32_t eb_timer_delete(eb_timer_t *timer);

#endif //__EB_PORT_H__