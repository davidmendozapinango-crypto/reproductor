#ifndef CANCION_H
#define CANCION_H


typedef struct Cancion { //Defino una canción como un nodo de lista doblemente enlazada.
    char nombre[100];
    struct Cancion *sig;
    struct Cancion *ant;
    float duracion;
} Cancion;

typedef struct ListaDoble {
    Cancion* cabeza; // Puntero al primer elemento
    Cancion* cola;   // Puntero al último elemento (facilita insertar al final)
    int tamano;   // Cantidad de elementos
} ListaDoble;

#endif /*CANCION_H*/