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

#ifndef __EVENT_WORKER_H__
#define __EVENT_WORKER_H__

#include "event_bus.h"

typedef struct eb_worker_t
{
    char name[EB_WORKER_MAX_NAME_LEN];
    eb_t *bus;
    eb_msg_t msg;
    eb_thread_t thread;
    eb_queue_t queue;
    uint32_t start_time;
    uint32_t index;
    bool running;
    bool cancelled;
    bool timer_enabled;
}eb_worker_t;

int32_t eb_worker_init(eb_t *bus);
int32_t eb_worker_exec(eb_t *bus, eb_sub_t *sub, uint32_t event_id, void *data, uint32_t len);
int32_t eb_worker_post(eb_t *bus, eb_evt_t *evt, uint8_t index, void *data, uint32_t len);
void eb_worker_timeout(eb_worker_t *worker);

#endif