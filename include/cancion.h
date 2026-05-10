#ifndef CANCION_H
#define CANCION_H
#include <string.h>

typedef struct Cancion { //Defino una canción como un nodo de lista doblemente enlazada.
    char nombre[100];
    struct Cancion *sig;
    struct Cancion *ant;
    float duracion;
} Cancion;

typedef struct ListaDoble {
    Cancion* cabeza;
    Cancion* cola;   
    int tamano;   
} ListaDoble;

typedef struct nodo_pila {
    int               value;
    struct nodo_pila *next;
} NodoPila;

typedef struct {
    NodoPila *tope;
} historial_pila;

#endif /*CANCION_H*/