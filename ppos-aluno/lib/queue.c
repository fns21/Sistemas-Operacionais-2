// PingPongOS - PingPong Operating System
// © Prof. Carlos A. Maziero, DINF UFPR
// Versão 2.1 -- 06/2026

// Este arquivo PODE/DEVE ser alterado.

// Implementação do TAD fila genérica

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "queue.h"

struct node_t
{
    void *item;
    struct node_t *next;
};

struct queue_t
{
    struct node_t *head;
    struct node_t *tail;
    struct node_t *iterator;
    int size;
};

struct queue_t *queue_create()
{
    struct queue_t *queue = malloc(sizeof(struct queue_t));
    if (queue == NULL)
        return NULL;

    queue->head     = NULL;
    queue->tail     = NULL;
    queue->iterator = NULL;
    queue->size     = 0;

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
        queue->head     = new_node;
        queue->tail     = new_node;
        queue->iterator = new_node;
    }
    else
    {
        queue->tail->next = new_node;
        queue->tail       = new_node;
    }
    queue->size++;

    return NOERROR;
}

int queue_del(struct queue_t *queue, void *item)
{
    if (queue == NULL || item == NULL)
        return ERROR;

    struct node_t *prev = NULL;
    struct node_t *curr = queue->head;

    // busca o node que contem o item
    while (curr != NULL)
    {
        if (curr->item == item)
            break;
        prev = curr;
        curr = curr->next;
    }

    if (curr == NULL)
        return ERROR;

    if (queue->iterator == curr)
        queue->iterator = curr->next;

    // remove o node da lista
    if (prev == NULL)
        queue->head = curr->next;  // era o head
    else
        prev->next = curr->next;

    if (curr == queue->tail)
        queue->tail = prev;  // era o tail

    free(curr);  // libera o node (nao o item)
    queue->size--;

    return NOERROR;
}

bool queue_has(struct queue_t *queue, void *item)
{
    if (queue == NULL || item == NULL)
        return false;

    struct node_t *curr = queue->head;
    while (curr != NULL)
    {
        if (curr->item == item)
            return true;
        curr = curr->next;
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
    return queue->iterator->item;
}

void *queue_next(struct queue_t *queue)
{
    if (queue == NULL || queue->size == 0 || queue->iterator == NULL)
        return NULL;

    queue->iterator = queue->iterator->next;

    if (queue->iterator == NULL)
        return NULL;

    return queue->iterator->item;
}

void *queue_item(struct queue_t *queue)
{
    if (queue == NULL || queue->size == 0 || queue->iterator == NULL)
        return NULL;

    return queue->iterator->item;
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
    struct node_t *curr = queue->head;
    while (curr != NULL)
    {
        if (func != NULL)
        {
            func(curr->item);
            printf(" ");
        }
        else
            printf("undef ");
        curr = curr->next;
    }
    printf("] (%d items)\n", queue->size);
}
