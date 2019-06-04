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

#include "event_bus_worker.h"

static struct event_bus_worker workers[MAX_NB_WORKERS];

static void event_worker_thread(void *arg)
{
    struct event_bus_worker *worker = (struct event_bus_worker *)arg;
    struct event_bus_ctx *bus = worker->msg.bus;
    struct event_bus_msg *msg = &worker->msg.data;
    uint32_t i;

    for(i = 0 ; i < MAX_NB_SUBSCRIBERS ; i++)
    {
        if(bus->subscribers[i].event_id == msg->event_id)
        {
            bus->subscribers[i].cb(msg->app_ctx, msg->arg);
        }
    }

    worker->thread_hdl = NULL;

    memset(worker->name, 0, sizeof(char) * EVT_WORKER_MAX_NAME_LEN);
    vTaskDelete(NULL);
}

int32_t event_worker_init(struct event_bus_ctx *bus)
{
    uint32_t i;

    for(i = 0 ; i < MAX_NB_WORKERS ; i++)
    {
        memset(&workers[i], 0, sizeof(struct event_bus_worker));
    }

    return 0;
}

int32_t event_worker_start(struct event_bus_ctx *bus, struct event_bus_msg *msg, bool wait)
{
    uint32_t i;
    struct event_bus_worker *worker;

    for(i = 0 ; i < MAX_NB_WORKERS ; i++)
    {
        worker = &workers[i];

        if(worker->thread_hdl == NULL)
        {
            worker->msg.bus = bus;
            sprintf(worker->name, "worker_%d_th", i);
            memcpy(&worker->msg.data, msg, sizeof(struct event_bus_msg));

            if(xTaskCreate(event_worker_thread, worker->name, EVT_WORKER_STACK_SIZE, (void *)worker, EVT_WORKER_PRIO, &worker->thread_hdl) != pdPASS)
            {
                return -1;
            }  

            return 0;
        }
    }

    return 0;
}