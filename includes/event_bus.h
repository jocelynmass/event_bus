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
#include "event_bus_dflt_cfg.h"
#include "event_bus_err.h"
#include "eb_port.h"

// Version 2.1
#define EVENT_BUS_MAJOR_REV     2
#define EVENT_BUS_MINOR_REV     1

typedef int32_t (eb_sub_cb_t)(void *app_ctx, uint32_t event_id, void *data, uint32_t len, void *arg);


struct eb_sub
{
    char name[EB_SUB_NAME_MAX_LEN];
    void *arg;
    bool direct;
    eb_sub_cb_t *cb;
};

struct eb_evt
{
    uint32_t id;
    uint32_t nb_sub;
    struct eb_sub subs[MAX_NB_SUBSCRIBERS];
};

struct eb_msg
{
    struct eb_evt *evt;
    uint32_t len;
    void *data;
};

struct eb_ctx
{
    uint32_t nb_evt;
    struct eb_evt events[MAX_NB_EVENTS];
    struct eb_sub all_sub;
    eb_mutex_t mutex;
    void *app_ctx;
};

int32_t eb_init(struct eb_ctx *bus, void *app_ctx);
int32_t eb_unsub(struct eb_ctx *bus, uint32_t event_id, eb_sub_cb_t *cb);
int32_t eb_sub_direct(struct eb_ctx *bus, const char *name, uint32_t event_id, void *arg, eb_sub_cb_t *cb);
int32_t eb_sub_indirect(struct eb_ctx *bus, const char *name, uint32_t event_id, void *arg, eb_sub_cb_t *cb);
int32_t eb_sub_all_direct(struct eb_ctx *bus, void *arg, eb_sub_cb_t *cb);
int32_t eb_sub_all_indirect(struct eb_ctx *bus, void *arg, eb_sub_cb_t *cb);
int32_t eb_pub(struct eb_ctx *bus, uint32_t event_id, void *data, uint32_t len);

#endif // __EVENT_BUS_H__