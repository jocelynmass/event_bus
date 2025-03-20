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

 #include "eb_zephyr.h"


static uint8_t thread_idx = 0;
static z_th_t eb_zephyr[CONFIG_EB_MAX_NB_THREADS];
static struct k_thread thread_pool[CONFIG_EB_MAX_NB_THREADS]; 
K_THREAD_STACK_ARRAY_DEFINE(stack_pool,	CONFIG_EB_MAX_NB_THREADS, CONFIG_EB_STACK_SIZE);
K_HEAP_ARRAY_DEFINE(heaps, CONFIG_EB_MAX_NB_THREADS, 1024)

static struct k_heap *eb_get_heap(void)
{
	k_tid_t tid = k_current_get();
	uint8_t i;

	for(i = 0 ; i < CONFIG_EB_MAX_NB_THREADS ; i++){
		if(eb_zephyr[i].tid == tid){
			return eb_zephyr[i].heap;
		}
	}

	return NULL;
}

int32_t eb_mutex_new(eb_mutex_t *mutex)
{
	return k_mutex_init(mutex);
}
 
int32_t eb_mutex_take(eb_mutex_t *mutex, uint32_t timeout)
{
	return k_mutex_lock(mutex, K_MSEC(timeout));
}
 
int32_t eb_mutex_give(eb_mutex_t *mutex)
{
    k_mutex_unlock(mutex);
    return 0;
}
 
int32_t eb_queue_new(eb_queue_t *queue, uint32_t item_size, uint32_t length)
{
	int32_t rc = 0;

	rc = k_msgq_alloc_init(queue, item_size, length);

	return rc;
}
 
int32_t eb_queue_push(eb_queue_t *queue, const void *item, uint32_t prio, uint32_t timeout)
{
	if(k_msgq_put(queue, item, K_MSEC(timeout)) != 0){
		return -1;
	}
    
	return 0;
}
 
int32_t eb_queue_get(eb_queue_t *queue, void *item, uint32_t timeout)
{
    return k_msgq_get(queue, item, K_MSEC(timeout));
}
 
 int32_t eb_queue_msg_waiting(eb_queue_t *queue)
 {
     return k_msgq_num_used_get(queue);
 }
 
 int32_t eb_queue_delete(eb_queue_t *queue)
 {
    k_msgq_cleanup(queue);
    return 0;
 }
 
eb_thread_t eb_thread_new(const char *name, eb_thread_func *thread, void *arg, int stack_size, int prio)
{
	z_th_t *z = &eb_zephyr[thread_idx];

	if(thread_idx >= CONFIG_EB_MAX_NB_THREADS){
		return NULL;
	}

	z->idx = thread_idx++;
	z->heap = heaps[z->idx];
	z->tid = k_thread_create(&thread_pool[z->idx], stack_pool[z->idx], CONFIG_EB_STACK_SIZE,
									thread, arg, NULL, NULL, prio, 0, K_NO_WAIT);
	
	k_thread_heap_assign(&thread_pool[z->idx], z->heap);
	k_thread_name_set(z->tid, name);
   	return z;
}
 
void eb_thread_delete(eb_thread_t thread)
{
	k_thread_abort(thread->tid);
}
 
uint32_t eb_get_tick(void)
{
	return k_uptime_get();
}
 
void *eb_malloc(size_t len)
{
	struct k_heap *h = eb_get_heap();

	if(h == NULL){
		return NULL;
	}

	return k_heap_alloc(h, len, K_NO_WAIT);
}
 
void eb_free(void *pmem)
{
	struct k_heap *h = eb_get_heap();

	if(h == NULL){
		return;
	}

	k_heap_free(h, pmem);
}
