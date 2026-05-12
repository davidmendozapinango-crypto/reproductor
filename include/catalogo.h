#ifndef CATALOGO_H
#define CATALOGO_H

#include "cancion.h"

/**
 * @brief Verifica si una cancion existe en el catalogo.
 * @param catalogo Lista que representa el catalogo musical.
 * @param nombre_cancion Nombre de la cancion a buscar.
 * @return 1 si existe, 0 si no existe o si los parametros son invalidos.
 */
int cancion_existe_en_catalogo(const ListaDoble *catalogo, const char *nombre_cancion);

#endif /* CATALOGO_H */
