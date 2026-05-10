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

void leerArchivo(char *nombre){ //Para leer el archivo e identificar catalogo y listas.
    FILE *ptrA = fopen(nombre, "r");
    if (ptrA == NULL) { //Verificación de que se pudo abrir el archivo
        perror("Error al abrir");
        return;
    }
    char linea[600];
    while (fgets(linea, sizeof(linea), ptrA)){
        if (strstr(linea, "Catálogo:") != NULL){

        }
    }
    {
        /* code */
    }
    

}
#endif /*CANCION_H*/