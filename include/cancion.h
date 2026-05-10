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
    Cancion* cabeza; // Puntero al primer elemento
    Cancion* cola;   // Puntero al último elemento (facilita insertar al final)
    int tamano;   // Cantidad de elementos
} ListaDoble;

void Menu(){//Imprimir menu
    printf("------MENU------\n - play (nombre de lista o cancion)\n - queue (nombre de lista o cancion)\n - new (nombre de lista): (nombre cancion) - (nombre cancion) \n - next \n - back \n - shuffle\n - loop\n - clear queue\n - clear history\n - catalog\n - lists\n - queue\n (q para salir)\n");

}

void leerArchivo(char *nombre){ //Para leer el archivo e identificar catalogo y listas.
    FILE *ptrA = fopen(nombre, "r");
    
    if (ptrA == NULL){
        printf("Error: No se pudo abrir el archivo '%s'\n", nombre);
        return;
    }

}
#endif /*CANCION_H*/