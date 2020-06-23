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

#include <stdlib.h>
#include "event_bus_worker.h"
#include "event_bus_supv.h"
#include "event_bus_stats.h"

static struct eb_worker workers[MAX_NB_WORKERS];

static void eb_worker_thread(void *arg);
static struct eb_worker *eb_worker_get(uint32_t *id);
static void eb_worker_timeout(void *arg);

static void eb_worker_timeout(void *arg)
{
    struct eb_worker *worker;
    struct eb_ctx *bus; 
    struct eb_msg *msg; 
    struct eb_evt *evt;

    worker = (struct eb_worker *)arg;
    worker->index++;
    worker->canceled = true;
    bus = worker->params.bus;
    msg = &worker->params.msg;
    evt = msg->evt;

    if(evt->nb_sub > worker->index)
    {
        eb_worker_process(bus, msg->evt, worker->index, msg->data, msg->len);
    }
}

static struct eb_worker *eb_worker_get(uint32_t *id)
{
    for(*id = 0 ; *id < MAX_NB_WORKERS ; (*id)++)
    {
        if(workers[*id].thread_hdl == 0)
        {
            return &workers[*id];
        }
    }

    return NULL;
}

static void eb_worker_thread(void *arg)
{
    struct eb_worker *worker = (struct eb_worker *)arg;
    struct eb_ctx *bus = worker->params.bus;
    struct eb_msg *msg = &worker->params.msg;
    struct eb_evt *evt = msg->evt;
    struct eb_sub *sub;
    uint32_t i;

    // Call all sub first
    sub = &bus->all_sub;

    if((sub->direct == false) && (sub->cb) && (worker->index == 0))
    {
        eb_worker_exec(bus, sub, evt->id, msg->data, msg->len);
    }
    
    for(i = worker->index ; i < evt->nb_sub ; i++)
    {
        sub = &evt->subs[i];
        
        if(sub->direct == false && sub->cb)
        {
            if(eb_supv_start(worker, eb_worker_timeout))
            {
                eb_log_warn("supervisor failed to start, no exec optimization available\n");
            }

            eb_worker_exec(bus, sub, evt->id, msg->data, msg->len);
            eb_supv_stop(worker);

            if(worker->canceled)
            {
                eb_log_trace("worker has been canceled\n");
                goto exit;
            }
        }
    }

exit:

    if(msg->data)
    {
        free(msg->data);
    }

    eb_thread_delete(worker->thread_hdl);
    memset(worker->name, 0, sizeof(worker->name));
    memset(worker, 0, sizeof(struct eb_worker)); 
}

int32_t eb_worker_init(struct eb_ctx *bus)
{
    memset(workers, 0, sizeof(workers));
    return 0;
}

int32_t eb_worker_exec(struct eb_ctx *bus, struct eb_sub *sub, uint32_t event_id, void *data, uint32_t len)
{
    uint32_t latency = 0;

    latency = eb_get_tick();
    sub->cb(bus->app_ctx, event_id, data, len, sub->arg);
    latency = eb_get_tick() - latency;
    eb_stats_add(bus, sub->name, event_id, latency);

    return 0;
}

int32_t eb_worker_process(struct eb_ctx *bus, struct eb_evt *evt, uint8_t index, void *data, uint32_t len)
{
    uint32_t id = 0;
    struct eb_worker *worker;

    worker = eb_worker_get(&id);

    if(worker == NULL)
    {
        eb_log_err("no workers available, drop event id 0x%lx\n", evt->id);
        return -1;
    }

    sprintf(worker->name, "wkr_%ld_th", id);
    worker->params.bus = bus;
    worker->params.msg.evt = evt;
    worker->index = index;

    if(len > 0)
    {
        worker->params.msg.data = malloc(len); //TODO: replace by a mempool alloc
        worker->params.msg.len = len;
        if(worker->params.msg.data == NULL)
        {
            eb_log_err("data alloc failed for event id 0x%lx\n", evt->id);
            return -1;
        }
        memcpy(worker->params.msg.data, data, len);
    }

    worker->thread_hdl = eb_thread_new(worker->name, eb_worker_thread, (void *)worker, EB_WORKER_STACK_SIZE, EB_WORKER_PRIO);

    if(worker->thread_hdl == NULL)
    {
        eb_log_err("%s failed\n", worker->name);
        return -1;
    }

    return 0;
}