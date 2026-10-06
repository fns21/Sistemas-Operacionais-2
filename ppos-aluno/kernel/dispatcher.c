// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// GRR20211782 Fabio Naconeczny da Silva

// Dispatcher: gerencia os estados das tarefas.
#include <stdlib.h>
#include <string.h>
#include "macros.h"
#include "dispatcher.h"
#include "scheduler.h"
#include "queue.h"
#include "task.h"
#include "time.h"
#include "hardware/cpu.h"

struct queue_t *task_ready_queue = NULL;

void dispatcher_init()
{
    task_ready_queue = queue_create();

    if (task_ready_queue == NULL)
    {
        ppos_debug("Erro ao criar fila de tarefas prontas");
        return;
    }
}

void dispatcher_term()
{
    if (task_ready_queue != NULL)
    {
        queue_destroy(task_ready_queue);
        task_ready_queue = NULL;
    }
    else
    {
        ppos_debug("Fila de tarefas prontas não existe");
        return;
    }
}

void dispatcher()
{
    extern void user_main(void *);

    task_t* task_user = task_create("user_main", user_main, NULL);
    if (task_user == NULL)
    {
        ppos_debug("Erro ao criar tarefa de usuário");
        return;
    }

    while(queue_size(task_ready_queue) > 0)
    {
        task_t *next_task = scheduler(task_ready_queue);
        if (next_task == NULL)
        {
            ppos_debug("Nenhuma tarefa pronta para executar");
            break;
        }

        task_run(next_task);

        switch (next_task->status)
        {
            case TASK_READY:
                // tarefa ja foi reinserida na fila por task_yield
                break;
            case TASK_SUSPENDED:
                break;
            case TASK_RUNNING:
                break;
            case TASK_TERMINATED:
                task_destroy(next_task);
                break;
            default:
                ppos_debug("Status de tarefa inválido");
                break;
        }
    }

    // relatório final do kernel
    printk("PPOS: task %3d (%s), %5u ms run, %5d ms cpu, %5d acts, exit code   0\n",
           task_kernel->id, task_kernel->name,
           time(), task_kernel->cpu_time, task_kernel->activations);
}

int task_switch(struct task_t *task)
{
    task_t *prev_task = current_task;

    // Finaliza e volta ao pai
    if (task == NULL)
    {
        prev_task->status = TASK_TERMINATED;
        task = current_task->parent;
    }

    // Ignora sem erro se a tarefa já tiver terminado
    if (task->status == TASK_TERMINATED)
        return NOERROR;

    // remove da fila de prontas se estiver lá
    queue_del(task_ready_queue, task);

    // Se nao terminou, volta para a fila de prontas e muda o status da tarefa atual
    if (prev_task != task_kernel && prev_task->status != TASK_TERMINATED)
        prev_task->status = TASK_READY;
    task->status = TASK_RUNNING;

    // Atualiza tarefa atual antes da troca de contexto
    current_task = task;

    // contabiliza ativações
    if (task == task_kernel)
        task_kernel->activations++;

    ppos_debug("task %d (%s) switch to task %d (%s)\n",
               prev_task->id, prev_task->name, task->id, task->name);

    hw_irq_enable(0); // evita contar o tempo de CPU da tarefa atual durante o ctx_switch
    ctx_switch(&prev_task->context, &task->context);
    hw_irq_enable(1);

    return NOERROR;
}

void task_run(struct task_t *task)
{
    if (task != NULL)
    {
        queue_del(task_ready_queue, task);
        task->status = TASK_RUNNING;
        task->quantum = QUANTUM; //reseta
        task_switch(task);
    }
}

void task_suspend(struct queue_t *queue)
{
    queue_del(task_ready_queue, current_task);
    
    current_task->status = TASK_SUSPENDED;

    if (queue != NULL)
        queue_add(queue, current_task);
    
    task_switch(task_kernel);
}

void task_awake(struct task_t *task)
{
    if (task != NULL && task->status == TASK_SUSPENDED)
    {
        queue_add(task_ready_queue, task);
        task->status = TASK_READY;
    }
}