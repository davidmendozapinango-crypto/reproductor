#ifndef CANCION_H
#define CANCION_H


typedef struct Cancion { //Defino una canción como un nodo de lista doblemente enlazada.
    char nombre[100];
    struct Cancion *sig;
    struct Cancion *ant;
    float duracion;
} Cancion;

#endif /*CANCION_H*/