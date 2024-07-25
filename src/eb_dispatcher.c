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
#include "event_bus.h"
#include "event_bus_worker.h"

static eb_worker_t *eb_dispatch_find_worker(eb_t *bus)
{
    eb_worker_t *worker = NULL;
    uint8_t id = 0;

    for(id = 0 ; id < MAX_NB_WORKERS ; id++){
        worker = eb_worker_get(id);
        if(worker->thread == NULL){
            if(eb_worker_new(bus, id) == EVT_BUS_ERR_OK){
                return worker;
            }
        }
        if(!worker->running){
            return worker;
        }
    }

    // No worker available, force push message to a running worker
    for(id = 0 ; id < MAX_NB_WORKERS ; id++){
        worker = eb_worker_get(id);
        if(eb_queue_msg_waiting(&worker->queue) < EB_WORKER_QUEUE_LEN){
            return worker;
        }
    }

    return NULL;
}


int32_t eb_dispatch(eb_t *bus, eb_msg_t *msg, bool indirect)
{
    eb_worker_t *w = NULL;
    eb_evt_t *evt = msg->evt;
    eb_sub_t *sub = NULL;
    int32_t rc = EVT_BUS_ERR_OK;
    uint8_t i = 0;

    if(msg->evt == NULL){
        rc = EVT_NULL_ERR;
        goto err;
    }

    if(indirect){
        w = eb_dispatch_find_worker(bus);
        if(w == NULL){
            rc = EVT_WORKER_ERR;
            goto err;
        }
        msg->retain++;
        eb_worker_post(bus, w, msg);
    }

    // Execute direct events
    for(i = 0 ; i < evt->nb_sub ; i++){
        sub = &evt->subs[i];

        if(sub->direct && sub->cb){
            eb_worker_exec(bus, sub, evt->id, msg->data, msg->len);
        }
    }
    
    // Handle all sub
    if(bus->all_sub.direct){
        eb_worker_exec(bus, &bus->all_sub, evt->id, msg->data, msg->len);
    }

err:
    // if msg is retain, the worker is then responsible for its destruction
    if((msg->data) && (msg->retain == 0)){
        eb_free(msg->data);
    }
}