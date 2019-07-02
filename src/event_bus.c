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

static void event_bus_thread(void *arg)
{
    struct event_bus_ctx *bus = (struct event_bus_ctx *)arg;
    struct event_bus_msg msg;

    do
    {
        if(xQueueReceive(bus->queue, &msg, portMAX_DELAY))
        {
            if(event_worker_start(bus, &msg, 0, false))
            {
                printf("[EVENT_BUS]: worker start faield\n");
            }
        }
    } while (1);
}

int32_t event_bus_init(struct event_bus_ctx *bus, void *app_ctx)
{
    uint32_t i;

    bus->sub_nb = 0;
    bus->app_ctx = app_ctx;

    for(i = 0 ; i < MAX_NB_SUBSCRIBERS ; i++)
    {
        memset(&bus->subscribers[i], 0, sizeof(struct event_bus_sub));
    }

    if(event_worker_init(bus))
        return EVT_WORKER_ERR;

    bus->queue = xQueueCreate(MAX_SIMLT_EVT, sizeof(struct event_bus_msg));

    if(bus->queue == NULL)
        return EVT_BUS_QUEUE_ERR;

    if(xTaskCreate(event_bus_thread, "evt_bus_th", EVT_BUS_STACK_SIZE, (void *)bus, EVT_BUS_PRIO, NULL) != pdPASS)
    {
        return EVT_BUS_THREAD_ERR;
    }     

    return 0;
}

int32_t event_bus_subscribe(struct event_bus_ctx *bus, uint32_t event_id, void *arg, int32_t (*sub_cb)(void *app_ctx, void *data, void *arg))
{
    struct event_bus_sub *sub;
    
    if(bus->sub_nb >= MAX_NB_SUBSCRIBERS)
        return EVT_BUS_MEM_ERR;

    sub = &bus->subscribers[bus->sub_nb];
    sub->event_id = event_id;
    sub->cb = sub_cb;
    sub->arg = arg;

    bus->sub_nb++;

    return EVT_BUS_ERR_OK;
}

int32_t event_bus_publish_generic(struct event_bus_ctx *bus, uint32_t event_id, void *data, bool is_isr)
{
    struct event_bus_msg msg;

    msg.event_id = event_id;
    msg.app_ctx = bus->app_ctx;
    msg.data = data;

    if(is_isr)
    {
        if(xQueueSendFromISR(bus->queue,( void * ) &msg, NULL) != pdTRUE)
            return -1;
    }else
    {
        if(xQueueSend(bus->queue,( void * ) &msg, portMAX_DELAY) != pdTRUE)
            return -1;
    }

    return 0;
}