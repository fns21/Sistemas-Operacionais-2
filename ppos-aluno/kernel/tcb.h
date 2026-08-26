// PingPongOS - PingPong Operating System
// © Prof. Carlos A. Maziero, DINF UFPR
// Versão 2.1 -- 06/2026

// Este arquivo PODE/DEVE ser alterado.

// Descritor de tarefas (TCB - Task Control Block).

#ifndef __PPOS_TCB__
#define __PPOS_TCB__

#include "ctx.h"

extern task_t *current_task;  // tarefa atual
extern task_t *kernel_task;   // tarefa do núcleo

enum task_status_t
{
    TASK_READY,
    TASK_RUNNING,
    TASK_WAITING,
    TASK_TERMINATED
};

// Task Control Block (TCB), infos sobre uma tarefa
typedef struct task_t
{
    int id;         // identificador da tarefa
    char *name;     // nome da tarefa
    ctx_t context;  // contexto da tarefa
    int status;     // pronta, executando, ...
    task_t *parent; // tarefa que a criou (NULL se for a tarefa kernel)
    ...             // demais informações, a completar
} task_t;

#endif
