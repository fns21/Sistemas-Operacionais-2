// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

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

void task_aging(struct queue_t *ready_queue)
{
    struct task_t *curr = queue_head(ready_queue);

    while (curr != NULL)
    {
        if (curr != current_task)
        {
            curr->prio_d += AGING_FACTOR;
            if (curr->prio_d > MAX_PRIO)
                curr->prio_d = MAX_PRIO;
        }
        curr = queue_next(ready_queue);
    }
}

struct task_t *scheduler(struct queue_t *ready_queue)
{
    if (ready_queue == NULL || queue_size(ready_queue) == 0)
        return NULL;

    struct task_t *best = queue_head(ready_queue);
    struct task_t *curr = queue_next(ready_queue);

    // encontra a tarefa com maior prioridade dinamica (ou menor id em caso de empate)
    while (curr != NULL)
    {
        if (curr->prio_d > best->prio_d || (curr->prio_d == best->prio_d && curr->id < best->id))
            best = curr;

        curr = queue_next(ready_queue);
    }

    task_aging(ready_queue); // aplica envelhecimento a todas as tarefas prontas, exceto a escolhida

    best->prio_d = best->prio_e; // reseta a prioridade dinamica da tarefa escolhida

    return best;
}

void sched_setprio(struct task_t *task, int prio)
{
    if (task == NULL)
        task = current_task;

    if (prio < MIN_PRIO || prio > MAX_PRIO)
        return;

    task->prio_e = prio;
    task->prio_d = prio;

}

int sched_getprio(struct task_t *task)
{   
    if(task == NULL)
        task = current_task;

    return task->prio_e;
}
