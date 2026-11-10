// PingPongOS - PingPong Operating System
// © Prof. Carlos A. Maziero, DINF UFPR
// Versão 2.1 -- 06/2026

// Este arquivo PODE/DEVE ser alterado.

// GRR20211782 Fabio Naconeczny da Silva

// Implementação do TAD Mapa de objetos

#include <stdio.h>
#include <stdlib.h>
#include "map.h"

struct map_t
{
    void **slots;  // objetos registrados (NULL indica ID livre)
    int size;      // numero de IDs do mapa
    int items;     // numero de IDs em uso
    int last;      // ultimo ID alocado (-1 se nenhum foi alocado)
};

// informa se o ID esta dentro dos limites do mapa
static int valid_id(struct map_t *map, int id)
{
    return id >= 0 && id < map->size;
}

struct map_t *map_create(int size)
{
    if (size <= 0)
        return NULL;

    struct map_t *map = malloc(sizeof(struct map_t));
    if (map == NULL)
        return NULL;

    map->slots = calloc(size, sizeof(void *));
    if (map->slots == NULL)
    {
        free(map);
        return NULL;
    }

    map->size  = size;
    map->items = 0;
    map->last  = -1;

    return map;
}

int map_destroy(struct map_t *map)
{
    if (map == NULL)
        return ERROR;

    free(map->slots);  // libera os slots (nao os objetos)
    free(map);
    return NOERROR;
}

int map_put(struct map_t *map, void *object)
{
    if (map == NULL || object == NULL || map->items == map->size)
        return ERROR;

    // busca circular a partir do ID seguinte ao ultimo alocado
    int id = map->last;
    do
        id = (id + 1) % map->size;
    while (map->slots[id] != NULL);

    map->slots[id] = object;
    map->last      = id;
    map->items++;

    return id;
}

void *map_get(struct map_t *map, int id)
{
    if (map == NULL || !valid_id(map, id))
        return NULL;

    return map->slots[id];
}

void *map_del(struct map_t *map, int id)
{
    if (map == NULL || !valid_id(map, id) || map->slots[id] == NULL)
        return NULL;

    void *object   = map->slots[id];
    map->slots[id] = NULL;
    map->items--;

    return object;
}

int map_items(struct map_t *map)
{
    if (map == NULL)
        return ERROR;
    return map->items;
}

int map_size(struct map_t *map)
{
    if (map == NULL)
        return ERROR;
    return map->size;
}

void map_print(char *name, struct map_t *map)
{
    printf("%s: ", name);

    if (map == NULL)
    {
        printf("undef\n");
        return;
    }

    printf("[ ");
    for (int id = 0; id < map->size; id++)
        printf("%c ", map->slots[id] != NULL ? '*' : '-');
    printf("] (%d/%d)\n", map->items, map->size);
}
