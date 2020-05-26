# Event Bus [![GitHub version](https://badge.fury.io/gh/jocelynmass%2Fevent_bus.svg)](https://badge.fury.io/gh/jocelynmass%2Fevent_bus) [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)



Event publisher/subscriber library allowing to reduce code dependencies. Events can be run inside the caller thread or in a dedicated thread, depending of the subscriber settings.

# Init

- Define event in event_bus_cstm.h (to be placed in your project directory)

```c
#include "event_bus_def.h"

enum event_bus_cstm_id
{
    EB_CUSTOM_EVT1 = EB_CUSTOM_OFFSET,
    EB_CUSTOM_EVT2,
};
```

- Init Event Bus

```c
void main(void)
{
    struct eb_ctx ebus;
    struct app_ctx app;
  
    eb_init(&ebus, &app);
    
    ...
}
```

# Direct API

This API allows to directly notify subscribers inside the publisher context. Subscribers will be notified sequentially, meaning timely critical calls can't be ensured as one subscriber can prevent the others to be executed.

![Direct API Diagram](docs/eb_direct.svg)

Usage:

- Create a subscriber

```c
static int32_t custom_evt1_sub(void *app_ctx, uint32_t event_id, void *data, uint32 len, void *arg)
{
    printf("received evt id = %d\n", event_id);
    return 0;
}

void foo(void)
{
    eb_sub_direct(&ebus, "custom_evt1", EB_CUSTOM_EVT1, NULL, custom_evt1_sub);
}
```

- Create a publisher

```c
void foo(void)
{
    eb_pub(&ebus, EB_CUSTOM_EVT1, NULL, 0);
}

```

# Indirect API

This API allows to indirectly notify subscribers. Event Bus will create a thread in which the subscribers will be called. In a case a subscriber would take too much time to be executed, the remaining subscribers would be defered to a new thread. This would ensure subscribers to be executed in a maximum known latency (EB_MAX_SUB_LATENCY_MS * number of subscribers). Priority based subscribers feature will be released soon.

![Direct API Diagram](docs/eb_indirect.svg)

Usage:

- Create a subscriber

```c
static int32_t custom_evt1_sub(void *app_ctx, uint32_t event_id, void *data, uint32 len, void *arg)
{
    printf("received evt id = %d\n", event_id);
    return 0;
}

void foo(void)
{
    eb_sub_indirect(&ebus, "custom_evt1", EB_CUSTOM_EVT1, NULL, custom_evt1_sub);
}
```

- Create a publisher

```c
void foo(void)
{
    eb_pub(&ebus, EB_CUSTOM_EVT1, NULL, 0);
}

```
