#ifndef UI_H
#define UI_H

/**
 * @file ui.h
 * @brief Interfaz de usuario basada en NCURSES para el reproductor musical.
 *
 * Proporciona la función principal para iniciar la experiencia de usuario tipo terminal,
 * permitiendo navegación con teclado y entrada de comandos predefinidos.
 *
 * Cumple el estándar de codificación CMU y utiliza NCURSES para la presentación.
 */

/**
 * @brief Inicia la interfaz de usuario con NCURSES.
 *
 * Controla la navegación, entrada de comandos y visualización de datos.
 * Separa la lógica de negocio de la presentación.
 */
void ui_main(void);

/**
 * @brief Valida si un comando pertenece al conjunto predefinido de UX.
 *
 * Acepta comandos exactos y comandos con prefijo valido para operaciones
 * que requieren argumento, por ejemplo play, queue, songs y new.
 *
 * @param comando Cadena de comando a validar.
 * @return 1 si el comando es valido para la UX, 0 en caso contrario.
 */
int ui_is_predefined_command(const char *comando);

#endif /* UI_H */
