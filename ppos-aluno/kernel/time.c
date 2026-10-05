// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// GRR20211782 Fabio Naconeczny da Silva

// Gerência básica do tempo.

#include "time.h"
#include "task.h"
#include "dispatcher.h"
#include "hardware/cpu.h"

// relógio do sistema 
static volatile unsigned int sys_clock = 0;

// handler do temporizador
static void timer_handler(int irq)
{
    sys_clock += TICK;

    if (current_task == NULL)
        return;

    current_task->cpu_time += TICK;

    // só preempta tarefas de usuário
    if (current_task == task_kernel)
        return;

    current_task->quantum--;

    // quantum esgotado: devolve a CPU ao dispatcher
    if (current_task->quantum <= 0)
        task_yield();
}

void time_init()
{
    sys_clock = 0;

    hw_irq_handle(IRQ_TIMER, timer_handler);

    hw_timer(TICK, TICK);
}

void time_term()
{
    hw_timer(0, 0);
    hw_irq_handle(IRQ_TIMER, NULL);
}

unsigned int time()
{
    return sys_clock;
}
