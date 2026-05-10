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