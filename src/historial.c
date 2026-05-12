#include "historial.h"

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
