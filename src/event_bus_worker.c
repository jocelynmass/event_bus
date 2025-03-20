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
#include "event_bus_stats.h"

static eb_worker_t workers[MAX_NB_WORKERS];

#ifdef WITH_ZEPHYR
static void eb_worker_thread(void *arg, void *arg2, void *arg3)
#else
static void eb_worker_thread(void *arg)
#endif
{
    eb_worker_t *worker = (eb_worker_t *)arg;
    eb_t *bus = worker->bus;
    eb_msg_t msg;
    eb_sub_t *sub;
    uint32_t i = 0;

    while(1){
        if(eb_queue_get(&worker->queue, &msg, EB_WORKER_QUEUE_PERIOD) == 0){
            worker->running = true;

            // Call all sub first
            if(!bus->all_sub.direct){
                eb_worker_exec(bus, &bus->all_sub, msg.evt->id, msg.data, msg.len);
            }

            for(i = 0 ; i < msg.evt->nb_sub ; i++){
                sub = &msg.evt->subs[i];
                if(!sub->direct){
                    eb_worker_exec(worker->bus, sub, msg.evt->id, msg.data, msg.len);
                }
            }
            
            if((msg.data) && (msg.retain == 1)){
                msg.retain--;
                eb_free(msg.data);
            }
            worker->running = false;
        }
    }
}

eb_worker_t *eb_worker_get(uint32_t id)
{
    if(id >= MAX_NB_WORKERS){
        return NULL;
    }

    return &workers[id];
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

int32_t eb_worker_new(eb_t *bus, uint32_t id)
{
    eb_worker_t *worker = NULL;
    int32_t rc = EVT_BUS_ERR_OK;

    worker = eb_worker_get(id);
    
    if(worker->thread == NULL){

        worker->bus = bus;
        worker->id = id;
        sprintf(worker->name, "wkr_%d_th", worker->id);

        if(eb_queue_new(&worker->queue, sizeof(eb_msg_t), EB_WORKER_QUEUE_LEN)){
            eb_log_err("%s queue failed\n", worker->name);
            rc = EVT_BUS_QUEUE_ERR;
            goto exit;
        }

        worker->thread = eb_thread_new(worker->name, eb_worker_thread, (void *)worker, EB_WORKER_STACK_SIZE, EB_WORKER_PRIO);
        if(worker->thread == NULL){
            eb_log_err("%s failed\n", worker->name);
            eb_queue_delete(&worker->queue);
            rc = EVT_BUS_THREAD_ERR;
            goto exit;
        }
    }

exit:
    return rc;
}   

int32_t eb_worker_post(eb_t *bus, eb_worker_t *worker, eb_msg_t *msg)
{   
    eb_queue_push(&worker->queue, (void *)msg, EVENT_BUS_LOW_PRIO, 100);
    return 0;
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