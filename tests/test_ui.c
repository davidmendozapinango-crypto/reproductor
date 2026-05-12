#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "../include/ui.h"

/*
 * Prueba básica: solo verifica que la función ui_main existe y puede ser llamada.
 * En un entorno real, se usaría automatización de terminal o mocks para validar la UI.
 */
int main(void)
{
    /* Solo invoca la función, el usuario debe salir con Ctrl+D. */
    printf("\n[TEST] Lanzando UI. Salga con Ctrl+D para finalizar la prueba manual.\n");
    ui_main();
    printf("\n[TEST] UI finalizada correctamente.\n");
    return 0;
}
