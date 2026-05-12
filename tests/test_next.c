#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "cancion.h"

void reproducir_cancion(ColeccionMusical *coleccion) {
    if (coleccion->lista_reproduccion.canciones.cabeza == NULL) {
        printf("Nada que reproducir.\n");
    } else {
        printf("Reproduciendo: %s\n", coleccion->lista_reproduccion.canciones.cabeza->nombre);
    }
}

void test_next() {
    ColeccionMusical coleccion;
    inicializar_coleccion_musical(&coleccion);

    // Crear canciones de prueba
    Cancion *cancion1 = crear_cancion("Cancion 1", 3.5);
    Cancion *cancion2 = crear_cancion("Cancion 2", 4.0);
    Cancion *cancion3 = crear_cancion("Cancion 3", 2.8);

    // Insertar canciones manualmente en la lista de reproducción
    coleccion.lista_reproduccion.canciones.cabeza = cancion1;
    cancion1->sig = cancion2;
    cancion2->ant = cancion1;
    cancion2->sig = cancion3;
    cancion3->ant = cancion2;
    coleccion.lista_reproduccion.canciones.cola = cancion3;

    // Reproducir la primera canción
    printf("Reproduciendo: %s\n", coleccion.lista_reproduccion.canciones.cabeza->nombre);

    // Avanzar a la siguiente canción
    next(&coleccion);
    assert(coleccion.lista_reproduccion.canciones.cabeza == cancion2);

    // Avanzar nuevamente
    next(&coleccion);
    assert(coleccion.lista_reproduccion.canciones.cabeza == cancion3);

    // Avanzar cuando no hay más canciones
    next(&coleccion);
    assert(coleccion.lista_reproduccion.canciones.cabeza == NULL);

    printf("Todas las pruebas de 'next' pasaron exitosamente.\n");
}

int main() {
    test_next();
    return 0;
}
