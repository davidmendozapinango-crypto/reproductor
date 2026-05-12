#include "utils.h"
#ifdef REPRODUCTOR_UX
#include "ui.h"
#endif

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
#ifdef REPRODUCTOR_UX
    ui_main();
#else
    imprimir_bienvenida();
    menu();
#endif
    return 0;
}
