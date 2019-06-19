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

#include "event_bus_supv.h"
#include "event_bus_worker.h"


int32_t event_supv_start(struct event_bus_worker *worker, void (* timeout_cb)(TimerHandle_t xTimer))
{
    if(worker->tmr_hdl != NULL)
        return -1;

    worker->tmr_hdl = xTimerCreate("evt_supv_tmr", EVT_MAX_SUB_LATENCY_MS, pdFALSE, (void *)worker, timeout_cb);

    if(worker->tmr_hdl == NULL)
        return -1;

    xTimerStart(worker->tmr_hdl, 0);

    return 0;
}

int32_t event_supv_stop(struct event_bus_worker *worker)
{
    xTimerStop(worker->tmr_hdl, 0);
	xTimerDelete(worker->tmr_hdl, 0);
    worker->tmr_hdl = NULL;
}