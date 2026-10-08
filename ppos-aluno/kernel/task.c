// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// GRR20211782 Fabio Naconeczny da Silva

// Gerência básica de tarefas.

#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <string.h>
#include "task.h"
#include "queue.h"
#include "dispatcher.h"
#include "macros.h"
#include "time.h"

#define STACK_SIZE 64 * 1024  // 64 KB por tarefa

task_t *current_task = NULL;
task_t *task_kernel = NULL;
static int next_id = 0;

extern struct queue_t *task_ready_queue;
struct queue_t *task_suspended_queue;

void task_init()
{
    task_kernel = malloc(sizeof(task_t));

    if (task_kernel == NULL)
    {
        ppos_debug("Erro ao alocar memória para a tarefa kernel");
        exit(1);
    }

    task_kernel->id          = next_id++;
    task_kernel->name        = "kernel";
    task_kernel->status      = TASK_RUNNING;
    task_kernel->parent      = NULL;
    task_kernel->prio_e      = DEFAULT_PRIO;
    task_kernel->prio_d      = task_kernel->prio_e;
    task_kernel->quantum     = QUANTUM; // quantum inicial
    task_kernel->cpu_time    = DEFAULT_CPU_TIME;
    task_kernel->activations = DEFAULT_ACTIVATIONS; // começa rodando
    task_kernel->waiting_for = NULL;
    task_kernel->exit_code   = 0;

    memset(&task_kernel->context, 0, sizeof(struct ctx_t));

    current_task = task_kernel;

    task_suspended_queue = queue_create();
    if (task_suspended_queue == NULL)
    {
        ppos_debug("Erro ao criar fila de tarefas suspensas");
        exit(1);
    }
}

void task_term()
{
    if (task_kernel != NULL)
    {
        free(task_kernel);
        task_kernel = NULL;
    }

    if (task_suspended_queue != NULL)
    {
        queue_destroy(task_suspended_queue);
        task_suspended_queue = NULL;
    }
}

task_t *task_create(char *name, void (*entry)(void *), void *arg)
{
    task_t *new_task = malloc(sizeof(task_t));
    if (new_task == NULL)
    {
        ppos_debug("Erro ao alocar memória para a nova tarefa");
        return NULL;
    }

    void *stack = NULL;
    if (posix_memalign(&stack, 16, STACK_SIZE) != 0)
        stack = NULL;
    if (stack == NULL)
    {
        ppos_debug("Erro ao alocar pilha para a nova tarefa");
        free(new_task);
        return NULL;
    }

    if (ctx_create(&new_task->context, entry, arg, stack, STACK_SIZE) < 0)
    {
        ppos_debug("Erro ao criar contexto da nova tarefa");
        free(stack);
        free(new_task);
        return NULL;
    }

    new_task->id          = next_id++;
    new_task->name        = name ? strdup(name) : NULL;
    new_task->status      = TASK_READY;
    new_task->parent      = current_task;
    new_task->prio_e      = DEFAULT_PRIO;
    new_task->prio_d      = new_task->prio_e;
    new_task->quantum     = QUANTUM; // quantum inicial
    new_task->cpu_time    = DEFAULT_CPU_TIME;
    new_task->activations = DEFAULT_ACTIVATIONS;
    new_task->waiting_for = NULL;
    new_task->exit_code   = 0;

    queue_add(task_ready_queue, new_task);

    ppos_debug("Task %s (ID %d) create task %s (ID %d)\n",
               current_task->name, current_task->id,
               new_task->name ? new_task->name : "(null)", new_task->id);

    return new_task;
}

int task_destroy(struct task_t *task)
{
    if (task == NULL || task->status != TASK_TERMINATED)
        return ERROR;

    ppos_debug("Task %s (ID %d) destroyed\n", task->name, task->id);

    // garante que a tarefa nao esta na fila de prontas
    queue_del(task_ready_queue, task);

    // libera a pilha salva no contexto
    if (task->context.stack != NULL)
        free(task->context.stack);

    free(task->name);
    free(task);

    return NOERROR;
}

int task_id(struct task_t *task)
{
    if (task == NULL)
        return current_task ? current_task->id : ERROR;
    return task->id;
}

char *task_name(struct task_t *task)
{
    if (task == NULL)
        return current_task ? current_task->name : NULL;
    return task->name;
}

void task_yield()
{
    current_task->status = TASK_READY;
    queue_add(task_ready_queue, current_task);
    current_task->activations++;
    task_switch(task_kernel);
}

void task_exit(int exit_code)
{
    ppos_debug("Task %s (ID %d) exited with code %d\n",
              current_task->name, current_task->id, exit_code);

    printk("PPOS: task %3d (%s), %5u ms run, %5d ms cpu, %5d acts, exit code %3d\n",
           current_task->id, current_task->name,
           time(), current_task->cpu_time, current_task->activations, exit_code);    

    current_task->status = TASK_TERMINATED;
    current_task->exit_code = exit_code;

    task_awake(current_task); // acorda tarefas que estavam esperando por esta tarefa

    task_switch(task_kernel);
}

int task_wait(struct task_t *task)
{
    if (task == NULL || task->status == TASK_TERMINATED)
        return 0; // encerra imediatamente sem suspender a tarefa atual

    ppos_debug("Task %s (ID %d) waiting for task %s (ID %d)\n",
               current_task->name, current_task->id,
               task->name, task->id);

    current_task->waiting_for = task;  // tarefa atual está esperando a tarefa "task" terminar

    // adiciona tarefa atual à fila global de tarefas suspensas 
    task_suspend(task_suspended_queue);

    return task->exit_code; // retorna o exit code da tarefa que terminou
}