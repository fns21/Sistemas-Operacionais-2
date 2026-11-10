# Teste do TAD mapa de objetos (lib/map.c)

# flags de compilação e ligação
CC       = gcc
CPPFLAGS = -I../../lib
CFLAGS   = -Wall -Wextra -g -std=c99
BIN      = map-test

# o TAD fica em lib/
vpath %.c ../../lib
vpath %.h ../../lib

# estes alvos não são arquivos
.PHONY: all clean purge

all: $(BIN)

# compila mapa de objetos
map-test:   map-test.o map.o
map-test.o: map-test.c map.h
map.o:      map.c map.h

# limpa arquivos temporários
clean:
	-rm -f *.o *~

# limpa tudo, deixa só o código-fonte
purge: clean
	-rm -f $(BIN)
