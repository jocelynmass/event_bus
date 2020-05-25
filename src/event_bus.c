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

static int32_t eb_lock(struct eb_ctx *bus)
{
    if(eb_mutex_take(&bus->p_mutex, 500))
        return 0;
    
    return -1;
}

static int32_t eb_unlock(struct eb_ctx *bus)
{
    if(eb_mutex_give(&bus->p_mutex))
        return 0;
    
    return -1;
}

static struct eb_evt *eb_search_event(struct eb_ctx *bus, uint32_t event_id)
{
    uint32_t i;

    for(i = 0 ; i < bus->nb_evt ; i++)
    {
        if(bus->events[i].id == event_id)
        {
            return &bus->events[i];
        }
    }

    return NULL;
}

static struct eb_evt *eb_get_event_ctx(struct eb_ctx *bus, uint32_t event_id)
{
    struct eb_evt *evt;

    evt = eb_search_event(bus, event_id);

    if(bus->nb_evt > MAX_NB_EVENTS)
    {
        return NULL;
    }

    if(evt == NULL)
    {
        evt = &bus->events[bus->nb_evt];
        bus->nb_evt++;
    }
    
    return evt;
}

static int32_t eb_subscribe(struct eb_ctx *bus, const char *name, bool direct, uint32_t event_id, void *arg, eb_sub_cb_t *cb)
{
    uint32_t i = 0;
    struct eb_evt *evt;
    struct eb_sub *sub;

    if(eb_lock(bus))
    {
        return EVT_BUS_LOCK_ERR;
    }

    evt = eb_get_event_ctx(bus, event_id);

    if(evt == NULL) 
    {
        return EVT_BUS_MEM_ERR;
    }

    evt->id = event_id;
    for(i = 0 ; i < evt->nb_sub ; i++)
    {
        sub = &evt->subs[i];
        if(sub->cb == cb)
        {
            eb_log_warn("subscriber already exists\n");
            goto exit;
        }
    }

    sub = &evt->subs[evt->nb_sub++];
    sub->cb = cb;
    sub->arg = arg;
    sub->direct = direct;
    
    memset(sub->name, 0, EB_SUB_NAME_MAX_LEN);
    strncpy(sub->name, name, MIN(strlen(name), EB_SUB_NAME_MAX_LEN-1));

exit:
    eb_unlock(bus);
    return EVT_BUS_ERR_OK;
}

static int32_t eb_subscribe_all(struct eb_ctx *bus, bool direct, void *arg, eb_sub_cb_t *cb)
{
    if(eb_lock(bus))
    {
        return EVT_BUS_LOCK_ERR;
    }

    strcpy(bus->all_sub.name, "all_sub");
    bus->all_sub.arg = arg;
    bus->all_sub.cb = cb;
    bus->all_sub.direct = direct;

    eb_unlock(bus);
    return EVT_BUS_ERR_OK;
}

static int32_t eb_publish_direct(struct eb_ctx *bus, struct eb_evt *evt, void *data, uint32_t len)
{
    uint32_t i;
    struct eb_sub *sub;

    if(evt == NULL)
    {
        return -1;
    }

    for(i = 0 ; i < evt->nb_sub ; i++)
    {
        sub = &evt->subs[i];

        if(sub->direct && sub->cb)
        {
            eb_worker_exec(bus, sub, evt->id, data, len);
        }
    }

    return -1;
}

static bool eb_is_indirect_sub(struct eb_ctx *bus, struct eb_evt *evt)
{
    uint32_t i;
    struct eb_sub *sub;

    if(evt == NULL)
    {
        return false;
    }

    for(i = 0 ; i < evt->nb_sub ; i++)
    {
        sub = &evt->subs[i];

        if(!sub->direct)
        {
            return true;
        }
    }

    return false;
}

int32_t eb_init(struct eb_ctx *bus, void *app_ctx)
{
    bus->nb_evt = 0;
    bus->app_ctx = app_ctx;

    eb_mutex_new(&bus->p_mutex);
    memset(bus->events, 0, sizeof(bus->events));
    memset(&bus->all_sub, 0, sizeof(bus->all_sub));

    if(eb_worker_init(bus) || bus->p_mutex == NULL)
        return EVT_WORKER_ERR;

    eb_log_trace("init done\n");
    return 0;
}

int32_t eb_unsub(struct eb_ctx *bus, uint32_t event_id, eb_sub_cb_t *cb)
{
    uint32_t i;
    bool found = false;
    struct eb_evt *evt;
    struct eb_sub *sub;

    if(eb_lock(bus))
    {
        return EVT_BUS_LOCK_ERR;
    }

    evt = eb_search_event(bus, event_id);

    if(evt == NULL)
    {
        goto exit;
    }

    for(i = 0 ; i < evt->nb_sub ; i++)
    {
        sub = &evt->subs[i];
        if(sub->cb == cb)
        {
            found = true;
            evt->nb_sub--;
        }

        if(found)
        {
            memcpy(sub, &evt->subs[i+1], sizeof(struct eb_sub));
            if(i == evt->nb_sub-1)
            {
                memset(&evt->subs[i+1], 0, sizeof(struct eb_sub));
            }
        }
    }

exit:
    eb_unlock(bus);
    return EVT_BUS_ERR_OK;
}

int32_t eb_sub_direct(struct eb_ctx *bus, const char *name, uint32_t event_id, void *arg, eb_sub_cb_t *cb)
{
    return eb_subscribe(bus, name, true, event_id, arg, cb);
}

int32_t eb_sub_indirect(struct eb_ctx *bus, const char *name, uint32_t event_id, void *arg, eb_sub_cb_t *cb)
{
    return eb_subscribe(bus, name, false, event_id, arg, cb);
}

int32_t eb_sub_all_direct(struct eb_ctx *bus, void *arg, eb_sub_cb_t *cb)
{
    return eb_subscribe_all(bus, true, arg, cb);
}

int32_t eb_sub_all_indirect(struct eb_ctx *bus, void *arg, eb_sub_cb_t *cb)
{
    return eb_subscribe_all(bus, false, arg, cb);
}

int32_t eb_pub(struct eb_ctx *bus, uint32_t event_id, void *data, uint32_t len)
{
    static struct eb_evt fake_evt;
    struct eb_evt *evt;
    struct eb_sub *sub;
    int32_t rc = 0;
    
    evt = eb_search_event(bus, event_id);
    
    sub = &bus->all_sub;
    if(sub->direct && sub->cb)
    {
        eb_worker_exec(bus, sub, event_id, data, len);
    }

    if(eb_is_indirect_sub(bus, evt) || (sub->direct == false))
    {
        // if evt == NULL this means we don't have any subscriber to this
        // event. We still need to call the all_sub cb, to do so we need to 
        // create a fake event with the current event id
        if(evt == NULL)
        {
            memset(&fake_evt, 0, sizeof(fake_evt));
            fake_evt.id = event_id;
            fake_evt.nb_sub = 0;
            evt = &fake_evt;
        }

        rc = eb_worker_process(bus, evt, 0, data, len);
    }
    
    rc |= eb_publish_direct(bus, evt, data, len);

    return rc;
}
            
            