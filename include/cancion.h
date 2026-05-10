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

#endif /*CANCION_H*/