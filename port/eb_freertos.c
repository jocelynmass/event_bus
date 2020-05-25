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

#include "eb_port.h"

int32_t eb_mutex_new(eb_mutex_t *mutex)
{
    *mutex = xSemaphoreCreateMutex();
   	
   	if(*mutex == NULL)
   		return -1;

    return 0;
}

int32_t eb_mutex_take(eb_mutex_t *mutex, uint32_t timeout)
{
   if(xSemaphoreTake(*mutex, timeout) == pdTRUE)
   		return 0;

    return -1;
}

int32_t eb_mutex_give(eb_mutex_t *mutex)
{
   	xSemaphoreGive(*mutex);

    return 0;
}

eb_thread_t eb_thread_new(const char *name, void (*thread)(void *arg), void *arg, int stack_size, int prio)
{
	eb_thread_t ret;

	if(xTaskCreate(thread, name, stack_size, arg, prio, &ret) != pdPASS)
	{
		return NULL;
    }

	return ret;
}

void eb_thread_delete(eb_thread_t thread)
{
    vTaskDelete(thread);
}

uint32_t eb_get_tick(void)
{
    return xTaskGetTickCount();
}

static void eb_timer_cb(TimerHandle_t xTimer)
{
    eb_timer_t *timer;

    // pvTimerGetTimerID return a pointer on the current running worker
    // This worker is taking to much time to proceed so we start a new worker
    // We need to ensure the current worker won't call the subscribers again when
    // the current callback will be completed. To do so we set the worker as canceled

    timer = (eb_timer_t *)pvTimerGetTimerID(xTimer);

    if(timer->cb)
    {
        timer->cb(timer->arg);
    }
}

int32_t eb_timer_new(eb_timer_t *timer, const char *name, uint32_t timeout_ms, void (*cb)(void *arg), void *arg)
{
    timer->cb = cb;
    timer->arg = arg;
    timer->hdl = xTimerCreate(name, timeout_ms, pdFALSE, (void *)timer, eb_timer_cb);

    if(timer->hdl == NULL)
        return -1;

    return 0;
}

int32_t eb_timer_start(eb_timer_t *timer)
{
    xTimerStart(timer->hdl, 0);

    return 0;
}

int32_t eb_timer_stop(eb_timer_t *timer)
{
    xTimerStop(timer->hdl, 0);

    return 0;
}

int32_t eb_timer_delete(eb_timer_t *timer)
{
    xTimerDelete(timer->hdl, 0);
    timer->hdl = NULL;
    timer->arg = NULL;
    timer->cb = NULL;

    return 0;
}
