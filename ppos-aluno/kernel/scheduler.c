// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Escalonador de tarefas prontas.

#include "scheduler.h"
#include "queue.h"

void sched_init()
{
}

void sched_term()
{
}

struct task_t *scheduler(struct queue_t *ready_queue)
{
    if (ready_queue == NULL || queue_size(ready_queue) == 0)
        return NULL;

    struct task_t *next_task = queue_head(ready_queue);
    queue_del(ready_queue, next_task);

    return next_task;
}

