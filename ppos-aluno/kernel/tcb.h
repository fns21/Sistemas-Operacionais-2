// PingPongOS - PingPong Operating System
// © Prof. Carlos A. Maziero, DINF UFPR
// Versão 2.1 -- 06/2026

// Este arquivo PODE/DEVE ser alterado.

// Descritor de tarefas (TCB - Task Control Block).

#ifndef __PPOS_TCB__
#define __PPOS_TCB__

#include "kernel/ctx.h"

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
    struct task_t *parent; // tarefa que a criou (NULL se for a tarefa kernel)
} task_t;

extern task_t *current_task;  // tarefa atual
extern task_t *task_kernel;   // tarefa do núcleo

#endif
