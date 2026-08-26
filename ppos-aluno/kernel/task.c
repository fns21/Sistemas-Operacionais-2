// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Gerência básica de tarefas.

#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <string.h>
#include "task.h"
#include "macros.h"

#define STACK_SIZE 64 * 1024  // 64 KB por tarefa

task_t *current_task = NULL;
static int next_id = 0;
task_t *task_kernel = NULL;

void task_init()
{
    task_kernel = malloc(sizeof(task_t));

    if (task_kernel == NULL)
    {
        ppos_debug("Erro ao alocar memória para a tarefa kernel");
        exit(1);
    }

    task_kernel->id     = next_id++;
    task_kernel->name   = "kernel";
    task_kernel->status = TASK_RUNNING;
    task_kernel->parent = NULL;

    memset(&task_kernel->context, 0, sizeof(struct ctx_t));

    current_task = task_kernel;
}

void task_term()
{
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

    new_task->id     = next_id++;
    new_task->name   = strdup(name);
    new_task->status = TASK_READY;
    new_task->parent = current_task;

    ppos_debug("Task %s (ID %d) create task %s (ID %d)\n",
               current_task->name, current_task->id, new_task->name, new_task->id);

    return new_task;
}

int task_destroy(struct task_t *task)
{
    if (task == NULL || task->status != TASK_TERMINATED)
        return ERROR;

    ppos_debug("Task %s (ID %d) destroyed\n", task->name, task->id);

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
