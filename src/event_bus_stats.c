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

#include "event_bus_stats.h"

static struct eb_hist stat_hist[EB_STAT_HIST_DEPTH];
static struct eb_stats stats;

int32_t eb_stats_init(struct eb_ctx *bus)
{
    memset(&stats, 0, sizeof(struct eb_stats));
    memset(stat_hist, 0, sizeof(struct eb_hist)*EB_STAT_HIST_DEPTH);

    return 0;
}

int32_t eb_stats_add(struct eb_ctx *bus, const char *name, uint32_t event_id, uint32_t latency)
{
    strcpy((char *)stat_hist[stats.index].name, name);
    stat_hist[stats.index].lat = latency;
    stat_hist[stats.index].event_id = event_id;

    if(stats.lat_min == 0 && stats.lat_avg == 0 && stats.lat_max == 0)
    {
        stats.lat_min = latency;
        stats.lat_avg = latency;
        stats.lat_max = latency;
    }else
    {
        if(latency < stats.lat_min)
            stats.lat_min = latency;

        if(latency > stats.lat_max)
        {
            stats.lat_max = latency;
            strcpy((char *)stats.lat_max_name, name);
        }

        stats.lat_avg = (stats.lat_avg + latency)/2;
    }

    stats.index++;
    if(stats.index >= EB_STAT_HIST_DEPTH)
    {
        stats.index = 0;
    }

    return 0;
}

void eb_stats_print(void)
{
    uint32_t i;

	printf("----> event bus stats:\n");
    printf("\t - version = %d.%d\n", EVENT_BUS_MAJOR_REV, EVENT_BUS_MINOR_REV);
    printf("\t - latency min = %ld ms\n", stats.lat_min);
    printf("\t - latency max = %ld ms\n", stats.lat_max);
	printf("\t - average latency = %ld ms\n", stats.lat_avg);
    printf("\t - max latency subscriber = %s\n", stats.lat_max_name);
    printf("\t - last events stats:\n");

    for(i = 0 ; i < EB_STAT_HIST_DEPTH ; i++)
    {
        printf("\t\t > subscriber: %s - event id = 0x%.8lx - latency = %ld ms\n", stat_hist[i].name, stat_hist[i].event_id, stat_hist[i].lat);
    }
}