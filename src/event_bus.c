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

static eb_evt_t *eb_get_event(eb_t *bus, uint32_t event_id);
static bool eb_has_indirect_sub(eb_t *bus, eb_evt_t *evt);
static int32_t eb_publish_direct(eb_t *bus, eb_evt_t *evt, void *data, uint32_t len);
static int32_t eb_publish_all(eb_t *bus, eb_evt_t *evt, uint32_t event_id, void *data, uint32_t len);

static int32_t eb_lock(eb_t *bus)
{
    if(eb_mutex_take(&bus->mutex, 500))
        return -1;
    
    return 0;
}

static int32_t eb_unlock(eb_t *bus)
{
    if(eb_mutex_give(&bus->mutex))
        return -1;;

    return 0;
}

static void eb_thread(void *arg)
{
    eb_t *bus = (eb_t *)arg;
    eb_evt_t *evt;
    eb_sub_t *sub;
    eb_msg_t msg;

    while(1){
        if(eb_queue_get(&bus->queue, &msg, EB_QUEUE_PERIOD) == 0){
            evt = eb_get_event(bus, msg.evt_id);
            msg.evt = evt;
            if(evt != NULL){
                if(eb_has_indirect_sub(bus, evt)){
                    eb_worker_post(bus, msg.evt, 0, msg.data, msg.len);
                }

                eb_publish_direct(bus, msg.evt, msg.data, msg.len); 
            }

            eb_publish_all(bus, msg.evt, msg.evt_id, msg.data, msg.len);
        }
        eb_supv_run();
    }
    
}

static eb_evt_t *eb_get_event(eb_t *bus, uint32_t event_id)
{
    uint32_t i;

    for(i = 0 ; i < bus->nb_evt ; i++){
        if(bus->events[i].id == event_id){
            return &bus->events[i];
        }
    }

    return NULL;
}

static eb_evt_t *eb_get_add_event(eb_t *bus, uint32_t event_id)
{
    eb_evt_t *evt;

    evt = eb_get_event(bus, event_id);

    // in the case the event has not been found, create a new one
    if(evt == NULL && bus->nb_evt < MAX_NB_EVENTS){
        evt = &bus->events[bus->nb_evt];
        memset(evt, 0, sizeof(eb_evt_t));
        bus->nb_evt++;
    }
    
    return evt;
}

static bool eb_sub_exists(eb_t *bus, eb_evt_t *evt, eb_sub_cb_t *cb)
{
    uint32_t i = 0;

    for(i = 0 ; i < evt->nb_sub ; i++){
        if(evt->subs[i].cb == cb){
            return true;
        }
    }

    return false;
}

static int32_t eb_subscribe(eb_t *bus, const char *name, bool direct, uint32_t event_id, void *arg, eb_sub_cb_t *cb)
{
    uint32_t i = 0;
    eb_evt_t *evt;
    eb_sub_t *sub;

    if(eb_lock(bus)){
        return EVT_BUS_LOCK_ERR;
    }

    evt = eb_get_add_event(bus, event_id);

    if(evt == NULL){
        return EVT_BUS_MEM_ERR;
    }

    evt->id = event_id;

    if(eb_sub_exists(bus, evt, cb)){
        goto exit;
    }

    sub = &evt->subs[evt->nb_sub++];
    memset(sub, 0, sizeof(eb_sub_t));
    sub->cb = cb;
    sub->arg = arg;
    sub->direct = direct;
    strncpy(sub->name, name, MIN(strlen(name), EB_SUB_NAME_MAX_LEN-1));

exit:
    eb_unlock(bus);
    return EVT_BUS_ERR_OK;
}

static int32_t eb_subscribe_all(eb_t *bus, bool direct, void *arg, eb_sub_cb_t *cb)
{
    if(eb_lock(bus)){
        return EVT_BUS_LOCK_ERR;
    }

    strcpy(bus->all_sub.name, "all_sub");
    bus->all_sub.arg = arg;
    bus->all_sub.cb = cb;
    bus->all_sub.direct = direct;

    eb_unlock(bus);
    return EVT_BUS_ERR_OK;
}

static int32_t eb_publish_direct(eb_t *bus, eb_evt_t *evt, void *data, uint32_t len)
{
    uint32_t i;
    eb_sub_t *sub;

    if(evt == NULL){
        return EVT_BUS_PUB_ERR;
    }

    for(i = 0 ; i < evt->nb_sub ; i++){
        sub = &evt->subs[i];

        if(sub->direct && sub->cb){
            eb_worker_exec(bus, sub, evt->id, data, len);
        }
    }

    return EVT_BUS_ERR_OK;
}

static int32_t eb_publish(eb_t *bus, eb_evt_t *evt, void *data, uint32_t len, uint32_t prio)
{
    eb_msg_t msg;

    msg.evt = evt;
    msg.len = len;

    if(len > 0){
        msg.data = eb_malloc(len); //TODO: replace by a mempool alloc
        if(msg.data == NULL){
            eb_log_err("data alloc failed for event id 0x%lx\n", evt->id);
            return EVT_BUS_ALLOC_ERR;
        }
        memcpy(msg.data, data, len);
    }

    if(eb_queue_push(&bus->queue, (void *)&msg, prio, EB_PUBLISH_TIMEOUT)){
        if(msg.data){
            eb_free(msg.data);
        }
        eb_log_err("failed to publish event id 0x%lx\n", evt->id);
        return EVT_BUS_PUB_ERR;
    }

    return EVT_BUS_ERR_OK;
}

static int32_t eb_publish_all(eb_t *bus, eb_evt_t *evt, uint32_t event_id, void *data, uint32_t len)
{
    static eb_evt_t fake_evt;

    if(bus->all_sub.direct){
        eb_worker_exec(bus, &bus->all_sub, event_id, data, len);
    }else{
        // if evt == NULL this means we don't have any subscriber to this
        // event. We still need to call the all_sub cb, to do so we need to 
        // create a fake event with the current event id
        if(evt == NULL){
            memset(&fake_evt, 0, sizeof(fake_evt));
            fake_evt.id = event_id;
            fake_evt.nb_sub = 0;
            evt = &fake_evt;
        }

        return eb_publish(bus, evt, data, len, EVENT_BUS_LOW_PRIO);
    }
    return 0;
}

static bool eb_has_indirect_sub(eb_t *bus, eb_evt_t *evt)
{
    uint32_t i;
    eb_sub_t *sub;

    if(evt == NULL){
        return false;
    }

    for(i = 0 ; i < evt->nb_sub ; i++){
        sub = &evt->subs[i];

        if(!sub->direct){
            return true;
        }
    }

    return false;
}

int32_t eb_unsub(eb_t *bus, uint32_t event_id, eb_sub_cb_t *cb)
{
    uint32_t i;
    bool found = false;
    eb_evt_t *evt;
    eb_sub_t *sub;

    if(eb_lock(bus)){
        return EVT_BUS_LOCK_ERR;
    }

    evt = eb_get_event(bus, event_id);

    if(evt == NULL){
        goto exit;
    }

    for(i = 0 ; i < evt->nb_sub ; i++){
        sub = &evt->subs[i];
        if(sub->cb == cb){
            found = true;
            evt->nb_sub--;
        }

        if(found){
            memcpy(sub, &evt->subs[i+1], sizeof(eb_sub_t));
            if(i == evt->nb_sub){
                memset(&evt->subs[i+1], 0, sizeof(eb_sub_t));
            }
        }
    }

exit:
    eb_unlock(bus);
    return EVT_BUS_ERR_OK;
}

int32_t eb_sub_direct(eb_t *bus, const char *name, uint32_t event_id, void *arg, eb_sub_cb_t *cb)
{
    return eb_subscribe(bus, name, true, event_id, arg, cb);
}

int32_t eb_sub_indirect(eb_t *bus, const char *name, uint32_t event_id, void *arg, eb_sub_cb_t *cb)
{
    return eb_subscribe(bus, name, false, event_id, arg, cb);
}

int32_t eb_sub_all_direct(eb_t *bus, void *arg, eb_sub_cb_t *cb)
{
    return eb_subscribe_all(bus, true, arg, cb);
}

int32_t eb_sub_all_indirect(eb_t *bus, void *arg, eb_sub_cb_t *cb)
{
    return eb_subscribe_all(bus, false, arg, cb);
}

int32_t eb_pub(eb_t *bus, uint32_t event_id, void *data, uint32_t len, uint32_t prio)
{
    eb_msg_t msg;
    int rc = EVT_BUS_ERR_OK;

    if(eb_lock(bus)){
        return EVT_BUS_LOCK_ERR;
    }
    
    msg.evt_id = event_id;
    msg.evt = NULL;
    msg.len = len;

    if(msg.len > 0){
        msg.data = eb_malloc(len); //TODO: replace by a mempool alloc
        if(msg.data == NULL){
            eb_log_err("data alloc failed for event id 0x%lx\n", event_id);
            rc = EVT_BUS_ALLOC_ERR;
            goto exit;
        }
        memcpy(msg.data, data, len);
    }

    if(eb_queue_push(&bus->queue, (void *)&msg, prio, EB_PUBLISH_TIMEOUT)){
        if(msg.data){
            eb_free(msg.data);
        }
        eb_log_err("failed to publish event id 0x%lx\n", event_id);
        rc = EVT_BUS_PUB_ERR;
        goto exit;
    }

exit:
    eb_unlock(bus);
    return rc;
}
            
int32_t eb_init(eb_t *bus, void *app_ctx)
{
    bus->nb_evt = 0;
    bus->app_ctx = app_ctx;
    
    if(eb_mutex_new(&bus->mutex)){
        return EVT_BUS_MUTEX_ERR;
    }

    memset(bus->events, 0, sizeof(eb_evt_t) *  MAX_NB_EVENTS);
    memset(&bus->all_sub, 0, sizeof(eb_sub_t));

    if(eb_queue_new(&bus->queue, sizeof(eb_msg_t), EB_QUEUE_LEN)){
        return EVT_BUS_QUEUE_ERR;
    }

    if(eb_thread_new("eb_th", eb_thread, (void *)bus, EB_STACK_SIZE, EB_PRIO) == NULL){
        return EVT_BUS_THREAD_ERR;
    }

    if(eb_worker_init(bus)){
        return EVT_WORKER_ERR;
    }

    eb_log_trace("init done\n");
    return 0;
}