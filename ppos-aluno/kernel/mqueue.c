// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// GRR20211782 Fabio Naconeczny da Silva

// Gerência de filas de mensagens

#include <stdlib.h>
#include <string.h>
#include "mqueue.h"
#include "semaphore.h"
#include "map.h"
#include "tcb.h"

#define MAX_MQUEUES 1024

typedef struct mqueue_t
{
    int id;                // identificador da fila
    int max_msgs;          // capacidade da fila, em mensagens
    int msg_size;          // tamanho de cada mensagem, em bytes
    int count;             // mensagens atualmente na fila
    int head, tail;        // posicoes de retirada e de insercao
    char *buffer;          // buffer circular com as mensagens
    int s_buffer;          // exclusao mutua no acesso ao buffer
    int s_item;            // mensagens disponiveis
    int s_vaga;            // vagas livres
} mqueue_t;

static struct map_t *mq_map = NULL; // filas existentes
static int mq_map_lock = 0;         // spinlock que protege o mapa

static void mq_free(mqueue_t *mq)
{
    sem_destroy(mq->s_vaga);
    sem_destroy(mq->s_item);
    sem_destroy(mq->s_buffer);

    preempt_lock = 1; // evita preempcao durante a liberacao
    free(mq->buffer);
    free(mq);
    preempt_lock = 0;
}

static int mq_transfer(int mqueue_id, void *msg, int send)
{
    if (msg == NULL)
        return ERROR;

    spin_lock(&mq_map_lock);

    mqueue_t *mq = map_get(mq_map, mqueue_id);
    if (mq == NULL)
    {
        spin_unlock(&mq_map_lock);
        return ERROR;
    }

    // guarda os IDs: a fila pode ser destruida enquanto a tarefa bloqueia
    int s_buffer = mq->s_buffer;
    int s_wait   = send ? mq->s_vaga : mq->s_item;
    int s_signal = send ? mq->s_item : mq->s_vaga;

    spin_unlock(&mq_map_lock);

    if (sem_down(s_wait) < 0 || sem_down(s_buffer) < 0)
        return ERROR;

    // com o s_buffer obtido, a fila existe: mqueue_destroy espera por ele
    if (send)
    {
        memcpy(mq->buffer + mq->tail * mq->msg_size, msg, mq->msg_size);
        mq->tail = (mq->tail + 1) % mq->max_msgs;
        mq->count++;
    }
    else
    {
        memcpy(msg, mq->buffer + mq->head * mq->msg_size, mq->msg_size);
        mq->head = (mq->head + 1) % mq->max_msgs;
        mq->count--;
    }

    sem_up(s_buffer);

    return sem_up(s_signal);
}

void mqueue_init()
{
    mq_map = map_create(MAX_MQUEUES);
}

void mqueue_term()
{
    for (int id = 0; id < map_size(mq_map); id++)
    {
        mqueue_t *mq = map_del(mq_map, id);
        if (mq != NULL)
            mq_free(mq);
    }

    map_destroy(mq_map);
    mq_map = NULL;
}

int mqueue_create(int max_msgs, int msg_size)
{
    if (max_msgs <= 0 || msg_size <= 0)
        return ERROR;

    preempt_lock = 1; // evita preempcao durante a alocacao

    mqueue_t *mq = malloc(sizeof(mqueue_t));
    char *buffer = calloc(max_msgs, msg_size);
    if (mq == NULL || buffer == NULL)
    {
        free(mq);
        free(buffer);
        preempt_lock = 0;
        return ERROR;
    }

    preempt_lock = 0;

    mq->max_msgs = max_msgs;
    mq->msg_size = msg_size;
    mq->count    = 0;
    mq->head     = 0;
    mq->tail     = 0;
    mq->buffer   = buffer;
    mq->s_buffer = sem_create(1);
    mq->s_item   = sem_create(0);
    mq->s_vaga   = sem_create(max_msgs);

    if (mq->s_buffer < 0 || mq->s_item < 0 || mq->s_vaga < 0)
    {
        mq_free(mq);
        return ERROR;
    }

    spin_lock(&mq_map_lock);
    int id = map_put(mq_map, mq);
    mq->id = id;
    spin_unlock(&mq_map_lock);

    if (id < 0)
    {
        mq_free(mq);
        return ERROR;
    }

    return id;
}

int mqueue_destroy(int mqueue_id)
{
    // retira do mapa antes de liberar: ninguem mais consegue encontra-la
    spin_lock(&mq_map_lock);
    mqueue_t *mq = map_del(mq_map, mqueue_id);
    spin_unlock(&mq_map_lock);

    if (mq == NULL)
        return ERROR;

    sem_down(mq->s_buffer); // espera quem estiver acessando o buffer
    mq_free(mq);

    return NOERROR;
}

int mqueue_send(int mqueue_id, void *msg)
{
    return mq_transfer(mqueue_id, msg, 1);
}

int mqueue_recv(int mqueue_id, void *msg)
{
    return mq_transfer(mqueue_id, msg, 0);
}

int mqueue_msgs(int mqueue_id)
{
    spin_lock(&mq_map_lock);

    mqueue_t *mq = map_get(mq_map, mqueue_id);
    // devolve o numero de mensagens na fila, ou ERROR se a fila nao existir
    int msgs = (mq != NULL) ? mq->count : ERROR;

    spin_unlock(&mq_map_lock);

    return msgs;
}
