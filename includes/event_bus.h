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

#ifndef __EVENT_BUS_H__
#define __EVENT_BUS_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h> 
#include <string.h> 
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "event_bus_dflt_cfg.h"
#include "event_bus_err.h"

struct event_bus_msg
{
    uint32_t event_id;
    void *app_ctx;
    void *data;
};

struct event_bus_sub
{
    uint32_t event_id;
    void *arg;
    int32_t (*cb)(void *app_ctx, void *data, void *arg);
};

struct event_bus_ctx
{
    uint32_t sub_nb;
    QueueHandle_t queue;
    struct event_bus_sub subscribers[MAX_NB_SUBSCRIBERS];
    void *app_ctx;
};

int32_t event_bus_init(struct event_bus_ctx *bus, void *app_ctx);
int32_t event_bus_subscribe(struct event_bus_ctx *bus, uint32_t event_id, void *arg, int32_t (*sub_cb)(void *app_ctx, void *data, void *arg));
int32_t event_bus_publish_generic(struct event_bus_ctx *bus, uint32_t event_id, void *data, bool is_isr);

#define event_bus_publish(ctx, event_id, data)  event_bus_publish_generic(ctx, event_id, data, false)
#define event_bus_publish_from_isr(ctx, event_id, data)  event_bus_publish_generic(ctx, event_id, data, true)

#endif // __EVENT_BUS_H__