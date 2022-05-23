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

#ifndef __EVENT_BUS_DFLT_CFG_H__
#define __EVENT_BUS_DFLT_CFG_H__

#include "eb_port.h"
#include "event_bus_cfg.h"

#ifndef MAX_NB_EVENTS
#define MAX_NB_EVENTS               16
#endif

#ifndef MAX_NB_SUBSCRIBERS
#define MAX_NB_SUBSCRIBERS          16
#endif

#ifndef MAX_NB_WORKERS
#define MAX_NB_WORKERS              4
#endif

#ifndef MAX_SIMLT_EVT
#define MAX_SIMLT_EVT               8
#endif

#ifndef EB_STACK_SIZE
#error "EB_STACK_SIZE must be set"
#endif

#ifndef EB_PRIO
#error "EB_STACK_SIZE must be set"
#endif

#ifndef EB_WORKER_STACK_SIZE
#error "EB_WORKER_STACK_SIZE must be set"
#endif

#ifndef EB_WORKER_PRIO
#error "EB_WORKER_PRIO must be set"
#endif

#ifndef EB_QUEUE_PERIOD
#define EB_QUEUE_PERIOD            (50)
#endif

#ifndef EB_WORKER_QUEUE_PERIOD
#define EB_WORKER_QUEUE_PERIOD     (2000)
#endif

#ifndef EB_PUBLISH_TIMEOUT
#define EB_PUBLISH_TIMEOUT         (200)
#endif

#ifndef EB_QUEUE_LEN
#define EB_QUEUE_LEN               (16)
#endif

#ifndef EB_WORKER_MAX_NAME_LEN
#define EB_WORKER_MAX_NAME_LEN     (16)
#endif

#ifndef EB_SUPV_MAX_TIMER
#define EB_SUPV_MAX_TIMER          (4)
#endif

#ifndef EB_MAX_SUB_LATENCY_MS
#define EB_MAX_SUB_LATENCY_MS      (100)
#endif

#ifndef EB_SUB_NAME_MAX_LEN
#define EB_SUB_NAME_MAX_LEN        (16)
#endif

#ifndef EB_STAT_HIST_DEPTH
#define EB_STAT_HIST_DEPTH         (4)
#endif

#ifndef EB_USE_CUSTOM_EVT
#endif

#ifndef EB_WORKER_EXIT_STATUS   
#define EB_WORKER_EXIT_STATUS       0
#endif

#ifndef MIN 
#define MIN(a,b)                   (a > b ? b : a)
#endif

#ifndef eb_log_trace
#define eb_log_trace(...)
#endif

#ifndef eb_log_warn
#define eb_log_warn(...)
#endif

#ifndef eb_log_err
#define eb_log_err(...)
#endif

#ifndef eb_log_evt
#define eb_log_evt(...)
#endif

#ifndef eb_get_tick_count
#define eb_get_tick_count
#endif

#endif // __EVENT_BUS_DFLT_CFG_H__