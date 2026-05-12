#include "catalogo.h"

#include <string.h>

/**
 * @brief Verifica si una cancion existe dentro del catalogo cargado.
 *
 * Recorre secuencialmente la lista doble del catalogo y compara el nombre
 * recibido contra cada cancion registrada. La comparacion es exacta y sensible
 * a mayusculas/minusculas segun strcmp.
 *
 * @param catalogo Puntero a la lista doble que contiene el catalogo musical.
 * @param nombre_cancion Nombre de la cancion que se desea buscar.
 * @return 1 si la cancion existe en el catalogo, 0 en caso contrario.
 */
int cancion_existe_en_catalogo(const ListaDoble *catalogo, const char *nombre_cancion)
{
	const Cancion *actual;

	if (catalogo == NULL || nombre_cancion == NULL)
	{
		return 0;
	}

	actual = catalogo->cabeza;
	while (actual != NULL)
	{
		if (strcmp(actual->nombre, nombre_cancion) == 0)
		{
			return 1;
		}
		actual = actual->sig;
	}

	return 0;
}
