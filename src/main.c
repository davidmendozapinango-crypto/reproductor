#include "utils.h"

/**
 * @brief Punto de entrada principal de la aplicacion de reproduccion musical.
 *
 * Inicializa la experiencia de consola mostrando el mensaje de bienvenida
 * y delega el ciclo interactivo al menu principal.
 *
 * @return 0 cuando la ejecucion finaliza correctamente.
 */
int main(void)
{
    imprimir_bienvenida();
    menu();
    return 0;
}
