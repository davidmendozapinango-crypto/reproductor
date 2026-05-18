#ifndef HISTORIAL_H
#define HISTORIAL_H

#include "cancion.h"

/**
 * @brief Libera todos los elementos del historial de reproduccion.
 * @param historial Pila de historial a limpiar.
 */
void limpiar_historial_reproduccion(historial_pila *historial);
void vaciar_historial_sistema(ColeccionMusical *coleccion);

#endif /* HISTORIAL_H */
