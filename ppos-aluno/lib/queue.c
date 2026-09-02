// PingPongOS - PingPong Operating System
// © Prof. Carlos A. Maziero, DINF UFPR
// Versão 2.1 -- 06/2026

// Este arquivo PODE/DEVE ser alterado.

// Implementação do TAD fila genérica

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "queue.h"

struct queue_t
{
    void* iterator;
    struct node_t* head;
    struct node_t* tail;
    int size;
}

struct node_t
{
    void* item;
    struct node_t* next;
};

struct queue_t *queue_create()
{
    struct queue_t *queue = malloc(sizeof(struct queue_t));
    if (queue == NULL)
        return NULL;

    queue->iterator = NULL;
    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;

    return queue;
}

int queue_destroy(struct queue_t *queue)
{    
    if (queue == NULL)
        return ERROR;

    free(queue);
    return NOERROR;
}

int queue_add(struct queue_t *queue, void *item)
{
    if (queue == NULL || item == NULL)
        return ERROR;

    struct node_t *new_node = malloc(sizeof(struct node_t));
    if (new_node == NULL)
        return ERROR;

    new_node->item = item;
    new_node->next = NULL;

    if (queue->size == 0)
    {
        queue->head = new_node;
        queue->tail = new_node;
        queue->iterator = new_node;
    }
    else
    {
        queue->tail->next = new_node;
        queue->tail = new_node;
    }
    queue->size++;

    return NOERROR;
}

int queue_del(struct queue_t *queue, void *item)
{
    if (queue == NULL || item == NULL)
        return ERROR;
    
    void* next_item = queue->head;
    while (next_item != queue->tail)
    {
        if (next_item == item)
        {
            if (queue->iterator == item)
                queue->iterator = queue_next(queue);
            free(item);
            queue->size--;
            return NOERROR;
        }
        next_item = queue_next(queue);
    }
    return ERROR;
}

bool queue_has(struct queue_t *queue, void *item)
{
    if (queue == NULL || item == NULL)
        return false;

    void* next_item = queue->head;
    while (next_item != queue->tail)
    {
        if (next_item == item)
            return true;
        next_item = queue_next(queue);
    }
    return false;
}

int queue_size(struct queue_t *queue)
{
    if (queue == NULL)
        return ERROR;
    return queue->size;
}

void *queue_head(struct queue_t *queue)
{
    if (queue == NULL || queue->size == 0)
        return NULL;

    queue->iterator = queue->head;
    return queue->iterator;
}

void *queue_next(struct queue_t *queue)
{
    if (queue == NULL || queue->size == 0)
        return NULL;

    if (queue->iterator == queue->tail)
        return NULL;

    queue->iterator = ((struct node_t*)queue->iterator)->next;
    return queue->iterator;
}

void *queue_item(struct queue_t *queue)
{
    if (queue == NULL || queue->size == 0)
        return NULL;

    return queue->iterator;
}

void queue_print(char *name, struct queue_t *queue, void(func)(void *))
{
    printf("%s: ", name);
    if (queue == NULL)
    {
        printf("undef\n");
        return;
    }

    if (queue->size == 0)
    {
        printf("[ ] (0 items)\n");
        return;
    }

    printf("[ ");
    void* item = queue_head(queue);
    while (item != NULL)
    {
        if (func != NULL)
            func(item);
        else
            printf("undef ");
        item = queue_next(queue);
    }
    printf("] (%d items)\n", queue->size);
}

