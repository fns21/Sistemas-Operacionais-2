// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Dispatcher: gerencia os estados das tarefas.
#include <stdlib.h>
#include <string.h>
#include "ctx.h"
#include "macros.h"
#include "task.h"
#include "dispatcher.h"

void dispatcher_init()
{
}

void dispatcher_term()
{
}

void dispatcher()
{
    extern void user_main(void *);

    task_t* task_user = task_create("user", user_main, NULL);
    if (task_user == NULL)
    {
        ppos_debug("Erro ao criar tarefa de usuário");
        exit(1);
    }

    task_switch(task_user);
    task_destroy(task_user);
}

int task_switch(struct task_t *task)
{
    task_t *prev_task = current_task;

    // Se task for NULL, transfere para a tarefa pai da atual
    if (task == NULL)
        task = current_task->parent;

    // Ignora sem erro se a tarefa já tiver terminado
    if (task->status == TASK_TERMINATED)
        return NOERROR;

    // Atualiza tarefa atual antes da troca de contexto
    current_task = task;

    ppos_debug("task %d (%s) switch to task %d (%s)\n",
               prev_task->id, prev_task->name, task->id, task->name);

    // Salva contexto da tarefa anterior e carrega o da próxima
    ctx_switch(&prev_task->context, &task->context);

    return NOERROR;
}