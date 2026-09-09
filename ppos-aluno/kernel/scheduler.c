// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// GRR20211782 Fabio Naconeczny da Silva

// Escalonador de tarefas prontas.

#include "tcb.h"
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

    // encontra tprox: tarefa com menor prio_d (menor = mais prioritario)
    struct task_t *best = queue_head(ready_queue);
    struct task_t *curr = queue_next(ready_queue);

    while (curr != NULL)
    {
        if (curr->prio_d < best->prio_d)
            best = curr;

        curr = queue_next(ready_queue);
    }

    // aplica aging em todas exceto a escolhida
    curr = queue_head(ready_queue);
    while (curr != NULL)
    {
        if (curr != best)
        {
            curr->prio_d += AGING_FACTOR;
            if (curr->prio_d < HIGH_PRIO)
                curr->prio_d = HIGH_PRIO;
        }
        curr = queue_next(ready_queue);
    }

    // reseta prio_d da escolhida
    best->prio_d = best->prio_e;

    return best;
}

void sched_setprio(struct task_t *task, int prio)
{
    if (task == NULL)
        task = current_task;

    if (prio > LOW_PRIO || prio < HIGH_PRIO)
        return;

    task->prio_e = prio;
    task->prio_d = prio;
}

int sched_getprio(struct task_t *task)
{
    if (task == NULL)
        task = current_task;

    return task->prio_e;
}
