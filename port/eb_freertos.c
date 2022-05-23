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
#include "event_bus.h"

int32_t eb_mutex_new(eb_mutex_t *mutex)
{
    *mutex = xSemaphoreCreateMutex();
   	
   	if(*mutex == NULL)
   		return -1;

    return 0;
}

int32_t eb_mutex_take(eb_mutex_t *mutex, uint32_t timeout)
{

    if(mcu_in_isr){
        if(xSemaphoreTakeFromISR(*mutex, NULL) == pdTRUE){
            return 0;
        }
    }else{
        if(xSemaphoreTake(*mutex, timeout) == pdTRUE){
   		    return 0;
        }
    }
    
    return -1;
}

int32_t eb_mutex_give(eb_mutex_t *mutex)
{
   	xSemaphoreGive(*mutex);

    return 0;
}

int32_t eb_queue_new(eb_queue_t *queue, uint32_t item_size, uint32_t length)
{
    *queue = xQueueCreate(length, item_size);
   	
   	if(*queue == NULL)
   		return -1;

    return 0;
}

int32_t eb_queue_push(eb_queue_t *queue, const void *item, uint32_t prio, uint32_t timeout)
{
    if(prio == EVENT_BUS_HIGH_PRIO){
        if(mcu_in_isr){
            if(xQueueSendToFront(*queue, item, timeout) == pdTRUE){
                return 0;
            }
        }else{
            if(xQueueSendToFrontFromISR(*queue, item, NULL) == pdTRUE){
                return 0;
            }
        }
    }else{
        if(mcu_in_isr){
            if(xQueueSendToBack(*queue, item, timeout) == pdTRUE){
                return 0;
            }
        }else{
            if(xQueueSendToBackFromISR(*queue, item, NULL) == pdTRUE){
                return 0;
            }
        }
    }
    return -1;
}

int32_t eb_queue_get(eb_queue_t *queue, void *item, uint32_t timeout)
{
    if(xQueueReceive(*queue, item, timeout) == pdPASS){
        return 0;
    }
    return -1;
}

int32_t eb_queue_delete(eb_queue_t *queue)
{
    if(queue){
        vQueueDelete(queue);
    }
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

void *eb_malloc(size_t len)
{
    return pvPortMalloc(len);
}

void eb_free(void *pmem)
{
    vPortFree(pmem);
}