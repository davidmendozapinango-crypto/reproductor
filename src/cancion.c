#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cancion.h"

void insertarCancion(Cancion **cabeza, char *nombre) {
    Cancion *nueva = (Cancion *)malloc(sizeof(Cancion));
    strncpy(nueva->nombre, nombre, 99);
    nueva->nombre[99] = '\0'; // Asegurar cierre de cadena
    nueva->sig = *cabeza;
    *cabeza = nueva;
}

void leerArchivo(char *nombre){ //Para leer el archivo e identificar catalogo y listas.
    FILE *ptrA = fopen(nombre, "r");
    if (ptrA == NULL) { //Verificación de que se pudo abrir el archivo
        perror("Error al abrir");
        return;
    }
    char linea[600];
    while (fgets(linea, sizeof(linea), ptrA)){
        if (strstr(linea, "Catálogo:") != NULL){
            char *token = strtok(linea, ":");
            token = strtok(NULL, " - \n\r");
            while (token != NULL){
                //LLAMADA A LA FUNCION DE NUEVA CANCION(MALLOC)
                printf("Cancion encontrada: %s\n", token);
                token = strtok(NULL, " - \n\r");
            }
        }
    }
}

Cancion* crearCancion(char* nombrePista, float duracionPista) {//crea nodo cancion
    Cancion* new = (Cancion*)malloc(sizeof(Cancion));
    if (new == NULL) {
        printf("Error: No hay memoria suficiente para crear la canción.\n");
        return NULL;
    }
    strncpy(nuevo->nombre, nombrePista, 99);
    nuevo->nombre[99] = '\0'; 
    nuevo->duracion = duracionPista;
    nuevo->sig = NULL;
    nuevo->ant = NULL;
    return new;
}