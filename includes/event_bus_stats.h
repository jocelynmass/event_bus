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

#ifndef __EVENT_BUS_STATS_H__
#define __EVENT_BUS_STATS_H__

#include "event_bus.h"

struct event_bus_hist
{
    char name[EVT_SUB_NAME_MAX_LEN];
    uint32_t event_id;
    uint32_t lat;
};

struct event_bus_stats
{
    uint32_t lat_min;
    uint32_t lat_avg;
    uint32_t lat_max;
    uint32_t index;
    char lat_max_name[EVT_SUB_NAME_MAX_LEN];
};

int32_t event_bus_stats_init(struct event_bus_ctx *bus);
int32_t event_bus_stats_add(struct event_bus_ctx *bus, char *name, uint32_t event_id, uint32_t latency);
void event_bus_stats_print(void);

#endif // __EVENT_BUS_STATS_H__