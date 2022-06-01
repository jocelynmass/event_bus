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

static eb_worker_t workers[MAX_NB_WORKERS];

void eb_worker_timeout(eb_worker_t *worker)
{
    worker->cancelled = true;
    if(worker->msg.evt->nb_sub > worker->index){
        eb_log_warn("worker timeout, defer event id %x to a new worker\n");
        eb_worker_post(worker->bus, worker->msg.evt, worker->index, worker->msg.data, worker->msg.len);
    }
}

static void eb_worker_thread(void *arg)
{
    eb_worker_t *worker = (eb_worker_t *)arg;
    eb_t *bus = worker->bus;
    eb_msg_t msg;
    eb_sub_t *sub;
    uint32_t i = 0;

    while(1){
        if(eb_queue_get(&worker->queue, &msg, EB_WORKER_QUEUE_PERIOD) == 0){
            worker->running = true;
            worker->cancelled = false;
            worker->msg.evt = msg.evt;
            worker->msg.data = NULL;
            worker->msg.len = msg.len;

            if(msg.len){
                worker->msg.data = msg.data;
            }

            // Call all sub first
            if(!bus->all_sub.direct && worker->index == 0){
                eb_worker_exec(bus, &bus->all_sub, msg.evt->id, msg.data, msg.len);
            }

            for(i = worker->index ; i < msg.evt->nb_sub ; i++){
                sub = &msg.evt->subs[i];
                if(!sub->direct){
                    eb_supv_start(worker);
                    worker->index++;
                    eb_worker_exec(worker->bus, sub, msg.evt->id, msg.data, msg.len);
                    if(worker->cancelled){
                        // worker has been cancelled, exit running state
                        break;
                    }
                }
            }

            // don't free data just yet in the case worker has been cancelled
            if(worker->msg.data && !worker->cancelled){
                eb_free(worker->msg.data);
            }
            worker->running = false;
        }
    }
}

static eb_worker_t *eb_worker_get(uint32_t *id)
{
    for(*id = 0 ; *id < MAX_NB_WORKERS ; (*id)++){
        if(!workers[*id].running){
            return &workers[*id];
        }
    }

    return NULL;
}

int32_t eb_worker_exec(eb_t *bus, eb_sub_t *sub, uint32_t event_id, void *data, uint32_t len)
{
    uint32_t latency = 0;

    latency = eb_get_tick();
    if(sub->cb){
        sub->cb(bus->app_ctx, event_id, data, len, sub->arg);
    }
    latency = eb_get_tick() - latency;
    eb_stats_add(bus, sub->name, event_id, latency);

    return 0;
}

int32_t eb_worker_post(eb_t *bus, eb_evt_t *evt, uint8_t index, void *data, uint32_t len)
{
    uint32_t id = 0;
    eb_worker_t *worker;
    eb_msg_t msg;
    int32_t rc = EVT_WORKER_ERR;

    worker = eb_worker_get(&id);
    if(worker == NULL){
        eb_log_err("no workers available, drop event id 0x%lx\n", evt->id);
        goto exit;
    }

    worker->bus = bus;
    worker->index = index;
    sprintf(worker->name, "wkr_%ld_th", id);

    if(worker->thread == NULL){
        if(eb_queue_new(&worker->queue, sizeof(eb_msg_t), 1)){
            eb_log_err("%s queue failed\n", worker->name);
            goto exit;
        }
        worker->thread = eb_thread_new(worker->name, eb_worker_thread, (void *)worker, EB_WORKER_STACK_SIZE, EB_WORKER_PRIO);
        if(worker->thread == NULL){
            eb_log_err("%s failed\n", worker->name);
            eb_queue_delete(&worker->queue);
            goto exit;
        }
    }

    msg.evt = evt;
    msg.len = len;
    msg.data = data;

    eb_queue_push(&worker->queue, (void *)&msg, EVENT_BUS_LOW_PRIO, 100);

exit:
    return rc;
}   

eb_worker_t *eb_worker_get_list(void)
{
    return workers;
}

int32_t eb_worker_init(eb_t *bus)
{
    memset(workers, 0, sizeof(workers));
    return 0;
}