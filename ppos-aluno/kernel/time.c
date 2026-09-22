// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// GRR20211782 Fabio Naconeczny da Silva

// Gerência básica do tempo.

#include "time.h"
#include "task.h"
#include "dispatcher.h"
#include "hardware/cpu.h"

// relógio do sistema em milissegundos
static volatile unsigned int sys_clock = 0;

// handler da IRQ do temporizador
static void timer_handler(int irq)
{
    // Desabilita IRQs imediatamente para evitar reentrância.
    // Como hw_irq_handle usa SA_NODEFER, o sinal não é mascarado
    // automaticamente durante o handler -- fazemos isso manualmente.
    hw_irq_enable(0);

    // incrementa o relógio do sistema a cada tick
    sys_clock += TICK;

    // só preempta tarefas de usuário (não preempta o kernel/dispatcher)
    if (current_task == NULL || current_task == task_kernel)
    {
        hw_irq_enable(1);
        return;
    }

    // decrementa o quantum da tarefa atual
    current_task->quantum--;

    if (current_task->quantum > 0)
    {
        // quantum ainda não esgotado: reabilita IRQs e retorna
        hw_irq_enable(1);
        return;
    }

    // quantum esgotado: devolve a CPU ao dispatcher.
    // task_yield vai chamar task_switch que já gerencia o hw_irq_enable
    // internamente (desabilita no início, reabilita após ctx_switch).
    // Aqui as IRQs já estão desabilitadas, então task_switch não
    // precisa desabilitar de novo (sigprocmask é idempotente).
    task_yield();
    // após o ctx_switch retornar (quando esta tarefa for retomada),
    // task_switch já reabilitou as IRQs.
}

void time_init()
{
    sys_clock = 0;

    // registra o handler do temporizador
    hw_irq_handle(IRQ_TIMER, timer_handler);

    // arma o timer para disparar após TICK ms e repetir a cada TICK ms
    hw_timer(TICK, TICK);
}

void time_term()
{
    // desarma o timer e remove o handler
    hw_timer(0, 0);
    hw_irq_handle(IRQ_TIMER, NULL);
}

unsigned int time()
{
    return sys_clock;
}
