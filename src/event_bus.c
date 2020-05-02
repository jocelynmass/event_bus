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


#include "event_bus.h"
#include "event_bus_worker.h"
#include "event_bus_stats.h"

static int32_t event_bus_lock(struct event_bus_ctx *bus)
{
    if(xSemaphoreTake(bus->mutex, 500))
        return 0;
    
    return -1;
}

static int32_t event_bus_unlock(struct event_bus_ctx *bus)
{
    if(xSemaphoreGive(bus->mutex))
        return 0;
    
    return -1;
}

int32_t event_bus_init(struct event_bus_ctx *bus, void *app_ctx)
{
    uint32_t i;

    bus->sub_nb = 0;
    bus->app_ctx = app_ctx;
    bus->mutex = xSemaphoreCreateMutex();

    for(i = 0 ; i < MAX_NB_SUBSCRIBERS ; i++)
    {
        memset(&bus->subscribers[i], 0, sizeof(struct event_bus_sub));
    }

    if(event_worker_init(bus) || bus->mutex == NULL)
        return EVT_WORKER_ERR;

    return 0;
}

int32_t event_bus_subscribe(struct event_bus_ctx *bus, const char *name, uint32_t event_id, void *arg, int32_t (*sub_cb)(void *app_ctx, void *data, void *arg))
{
    uint32_t i;
    struct event_bus_sub *sub;
    
    if(bus->sub_nb >= MAX_NB_SUBSCRIBERS)
        return EVT_BUS_MEM_ERR;

    if(event_bus_lock(bus))
    {
        return EVT_BUS_LOCK_ERR;
    }

    for(i = 0 ; i < bus->sub_nb ; i++)
    {
        sub = &bus->subscribers[i];
        if(sub_cb == sub->cb)
        {
            printf("[EVENT_BUS]: subscriber already exists\n");
            goto exit;
        }
    }

    sub = &bus->subscribers[bus->sub_nb];
    sub->event_id = event_id;
    sub->cb = sub_cb;
    sub->arg = arg;

    memset(sub->name, 0, EVT_SUB_NAME_MAX_LEN);

    if(strlen(name) > EVT_SUB_NAME_MAX_LEN)
        strncpy(sub->name, name, EVT_SUB_NAME_MAX_LEN-1);
    else
        strcpy(sub->name, name);

    bus->sub_nb++;

exit:
    event_bus_unlock(bus);
    return EVT_BUS_ERR_OK;
}

int32_t event_bus_unsubscribe(struct event_bus_ctx *bus, int32_t (*sub_cb)(void *app_ctx, void *data, void *arg))
{
    uint32_t i;
    int32_t pos = -1;
    bool found = false;
    struct event_bus_sub *sub;
    
    if(bus->sub_nb >= MAX_NB_SUBSCRIBERS)
        return EVT_BUS_MEM_ERR;

    if(event_bus_lock(bus))
    {
        return EVT_BUS_LOCK_ERR;
    }

    for(i = 0 ; i < bus->sub_nb ; i++)
    {
        sub = &bus->subscribers[i];
        if(!found && (sub_cb == sub->cb))
        {
            found = true;
             bus->sub_nb--;
        }

        if(found)
        {
            memcpy(sub, &bus->subscribers[i+1], sizeof(struct event_bus_sub));
            if(i == bus->sub_nb-1)
            {
                 memset(&bus->subscribers[i+1], 0, sizeof(struct event_bus_sub));
            }
        }
    }

    event_bus_unlock(bus);
    return EVT_BUS_ERR_OK;
}

int32_t event_bus_publish(struct event_bus_ctx *bus, uint32_t event_id, void *data)
{
    struct event_bus_msg msg;

    msg.event_id = event_id;
    msg.app_ctx = bus->app_ctx;
    msg.data = data;

    if(event_worker_start(bus, &msg, 0, false))
    {
        printf("[EVENT_BUS]: worker start faield\n");
    }

    return 0;
}

int32_t event_bus_publish_direct(struct event_bus_ctx *bus, uint32_t event_id, void *data)
{
    uint32_t i, latency = 0;
    struct event_bus_sub *sub;

    for(i = 0 ; i < bus->sub_nb ; i++)
    {
        sub = &bus->subscribers[i];

        if(sub->event_id == event_id)
        {
            latency = xTaskGetTickCount();
            sub->cb(bus->app_ctx, data, sub->arg);
            latency = xTaskGetTickCount() - latency;
            event_bus_stats_add(bus, sub->name, event_id, latency);
            return 0;
        }
    }

    return -1;
}
            
            