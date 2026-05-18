#include "historial.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Libera todos los nodos del historial de reproduccion y deja la pila vacia.
 *
 * Recorre la pila enlazada desde el tope hasta el final, libera cada nodo
 * en memoria dinamica y finalmente establece el tope en NULL para dejar
 * el historial en un estado consistente.
 *
 * @param historial Puntero a la pila de historial que se desea limpiar.
 *                  Si es NULL, la funcion no realiza ninguna accion.
 */
void limpiar_historial_reproduccion(historial_pila *historial)
{
	NodoPila *actual;

	if (historial == NULL)
	{
		return;
	}

	actual = historial->tope;
	while (actual != NULL)
	{
		NodoPila *siguiente;

		siguiente = actual->next;
		free(actual);
		actual = siguiente;
	}

	historial->tope = NULL;
}

void vaciar_historial_sistema(ColeccionMusical *coleccion)
{
    if (coleccion == NULL) 
    {
        return;
    }
    
    // Accedemos de forma nativa y segura:
    // Entramos a coleccion -> luego a lista_reproduccion -> y tomamos la direccion de su historial
    limpiar_historial_reproduccion(&coleccion->lista_reproduccion.historial);
    
    printf("Historial de reproduccion vaciado correctamente.\n");
}