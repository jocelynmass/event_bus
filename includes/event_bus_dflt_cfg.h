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

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "event_bus_cfg.h"

#ifndef MAX_NB_SUBSCRIBERS
#define MAX_NB_SUBSCRIBERS          16
#endif

#ifndef MAX_NB_WORKERS
#define MAX_NB_WORKERS              4
#endif

#ifndef MAX_SIMLT_EVT
#define MAX_SIMLT_EVT               8
#endif

#ifndef EVT_BUS_STACK_SIZE
#define EVT_BUS_STACK_SIZE          (configMINIMAL_STACK_SIZE * 4)
#endif

#ifndef EVT_BUS_PRIO
#define EVT_BUS_PRIO                (tskIDLE_PRIORITY + 1)
#endif

#ifndef EVT_WORKER_STACK_SIZE
#define EVT_WORKER_STACK_SIZE       (configMINIMAL_STACK_SIZE * 4)
#endif

#ifndef EVT_WORKER_PRIO
#define EVT_WORKER_PRIO             (tskIDLE_PRIORITY + 1)
#endif

#ifndef EVT_WORKER_MAX_NAME_LEN
#define EVT_WORKER_MAX_NAME_LEN     (16)
#endif

#ifndef EVT_SUPV_MAX_TIMER
#define EVT_SUPV_MAX_TIMER          (4)
#endif

#ifndef EVT_MAX_SUB_LATENCY_MS
#define EVT_MAX_SUB_LATENCY_MS      (100)
#endif

#endif // __EVENT_BUS_DFLT_CFG_H__