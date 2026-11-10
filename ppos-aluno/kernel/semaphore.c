// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// GRR20211782 Fabio Naconeczny da Silva

// Semáforos e spinlocks

#include <stdlib.h>
#include "semaphore.h"
#include "dispatcher.h"
#include "map.h"
#include "queue.h"
#include "tcb.h"

#define MAX_SEMAPHORES 1024

typedef struct semaphore_t
{
    int id;                // identificador do semaforo
    int value;             // contador
    struct queue_t *queue; // tarefas bloqueadas
    int lock;              // spinlock que protege o semaforo
} semaphore_t;

static struct map_t *sem_map = NULL; // semaforos existentes
static int sem_map_lock = 0;         // spinlock que protege o mapa

extern struct queue_t *task_ready_queue;

void spin_lock(int *lock)
{
    while (__sync_lock_test_and_set(lock, 1))
        ;
}

void spin_unlock(int *lock)
{
    __sync_lock_release(lock);
}

static semaphore_t *sem_get_locked(int sem_id)
{
    spin_lock(&sem_map_lock);

    semaphore_t *sem = map_get(sem_map, sem_id);
    if (sem != NULL)
        spin_lock(&sem->lock);

    spin_unlock(&sem_map_lock);

    return sem;
}

static void sem_wake_first(semaphore_t *sem, int result)
{
    task_t *task = queue_head(sem->queue);

    queue_del(sem->queue, task);
    task->sem_result = result;
    task->status = TASK_READY;
    queue_add(task_ready_queue, task);
}

void sem_init()
{
    sem_map = map_create(MAX_SEMAPHORES);
}

void sem_term()
{
    for (int id = 0; id < map_size(sem_map); id++)
    {
        semaphore_t *sem = map_del(sem_map, id);
        if (sem != NULL)
        {
            queue_destroy(sem->queue);
            free(sem);
        }
    }

    map_destroy(sem_map);
    sem_map = NULL;
}

int sem_create(int value)
{
    if (value < 0)
        return ERROR;

    preempt_lock = 1; // evita preempcao durante a alocacao

    semaphore_t *sem = malloc(sizeof(semaphore_t));
    struct queue_t *queue = queue_create();
    if (sem == NULL || queue == NULL)
    {
        free(sem);
        queue_destroy(queue);
        preempt_lock = 0;
        return ERROR;
    }

    preempt_lock = 0;

    sem->value = value;
    sem->queue = queue;
    sem->lock  = 0;

    spin_lock(&sem_map_lock);
    int id = map_put(sem_map, sem);
    sem->id = id;
    spin_unlock(&sem_map_lock);

    if (id < 0)
    {
        preempt_lock = 1;
        queue_destroy(queue);
        free(sem);
        preempt_lock = 0;
        return ERROR;
    }

    return id;
}

int sem_destroy(int sem_id)
{
    // retira do mapa antes de travar
    spin_lock(&sem_map_lock);

    semaphore_t *sem = map_del(sem_map, sem_id);
    if (sem == NULL)
    {
        spin_unlock(&sem_map_lock);
        return ERROR;
    }

    spin_lock(&sem->lock); // espera quem estiver operando no semaforo
    spin_unlock(&sem_map_lock);

    preempt_lock = 1;

    while (queue_size(sem->queue) > 0)
        sem_wake_first(sem, ERROR);

    queue_destroy(sem->queue);
    free(sem);

    preempt_lock = 0;

    return NOERROR;
}

int sem_down(int sem_id)
{
    semaphore_t *sem = sem_get_locked(sem_id);
    if (sem == NULL)
        return ERROR;

    sem->value--;

    if (sem->value < 0)
    {
        // sem preempcao, nenhuma tarefa roda entre soltar o spinlock e
        // entrar na fila; o sem_up nao perde este bloqueio
        preempt_lock = 1;
        spin_unlock(&sem->lock);
        task_suspend(sem->queue);

        // o semaforo pode nao existir mais aqui
        return current_task->sem_result;
    }

    spin_unlock(&sem->lock);

    return NOERROR;
}

int sem_up(int sem_id)
{
    semaphore_t *sem = sem_get_locked(sem_id);
    if (sem == NULL)
        return ERROR;

    sem->value++;

    if (sem->value <= 0)
    {
        preempt_lock = 1;
        sem_wake_first(sem, NOERROR);
        preempt_lock = 0;
    }

    spin_unlock(&sem->lock);

    return NOERROR;
}
