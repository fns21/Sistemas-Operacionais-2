// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// GRR20211782 Fabio Naconeczny da Silva

// Sistema produtor/consumidor com buffer limitado

#include <assert.h>
#include "lib/pplibc.h"
#include "syscall.h"

#define BUFFER_SIZE 5

static int buffer[BUFFER_SIZE];     // fila circular de itens
static int inicio = 0, fim = 0;     // posicoes de retirada e de insercao
static int s_buffer, s_item, s_vaga;

// corpo dos produtores
void produtor(void *arg)
{
    char *name = task_name(NULL);

    while (1)
    {
        task_sleep(1000);
        int item = randnum() % 100;

        sem_down(s_vaga);

        sem_down(s_buffer);
        buffer[fim] = item;
        fim = (fim + 1) % BUFFER_SIZE;
        printk("%s produziu %d\n", name, item);
        sem_up(s_buffer);

        sem_up(s_item);
    }
}

// corpo dos consumidores
void consumidor(void *arg)
{
    char *name = task_name(NULL);

    while (1)
    {
        sem_down(s_item);

        sem_down(s_buffer);
        int item = buffer[inicio];
        inicio = (inicio + 1) % BUFFER_SIZE;
        sem_up(s_buffer);

        sem_up(s_vaga);

        printk("                             %s consumiu %d\n", name, item);
        task_sleep(1000);
    }
}

// corpo da tarefa principal
void user_main(void *arg)
{
    s_buffer = sem_create(1);
    assert(s_buffer >= 0);
    s_item = sem_create(0);
    assert(s_item >= 0);
    s_vaga = sem_create(BUFFER_SIZE);
    assert(s_vaga >= 0);

    assert(task_create("p1", produtor, NULL));
    assert(task_create("p2", produtor, NULL));
    assert(task_create("p3", produtor, NULL));
    assert(task_create("c1", consumidor, NULL));
    assert(task_create("c2", consumidor, NULL));

    task_exit(NOERROR);
}
