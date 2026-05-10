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
}

Cancion* crearCancion(char* nombrePista, float duracionPista) {
    // 1. Reservar memoria dinámica para el nuevo nodo
    Cancion* new = (Cancion*)malloc(sizeof(Cancion));

    // 2. Verificar que la memoria se asignó correctamente (Manejo dinámico) 
    if (new == NULL) {
        printf("Error: No hay memoria suficiente para crear la canción.\n");
        return NULL;
    }

    // 3. Copiar los datos al nodo
    // Usamos strncpy para no desbordar el arreglo de 100 caracteres 
    strncpy(nuevo->nombre, nombrePista, 99);
    nuevo->nombre[99] = '\0'; // Asegurar el cierre del string
    nuevo->duracion = duracionPista;

    // 4. Inicializar punteros como NULL (VITAL en Listas Dobles) 
    nuevo->sig = NULL;
    nuevo->ant = NULL;

    return nuevo;
}

#endif /*CANCION_H*/