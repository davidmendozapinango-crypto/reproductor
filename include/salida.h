#ifndef SALIDA_H
#define SALIDA_H

#include <stdarg.h>

typedef enum SalidaModo {
    SALIDA_MODO_CLI = 0,
    SALIDA_MODO_UX = 1
} SalidaModo;

typedef enum SalidaNivel {
    SALIDA_NIVEL_INFO = 0,
    SALIDA_NIVEL_ERROR = 1
} SalidaNivel;

typedef void (*SalidaUxHandler)(SalidaNivel nivel, const char *mensaje, void *userdata);

/**
 * @brief Configura el modo global de salida para la aplicacion.
 *
 * @param modo Modo destino (CLI o UX).
 */
void salida_set_modo(SalidaModo modo);

/**
 * @brief Obtiene el modo global de salida actualmente activo.
 *
 * @return Modo de salida configurado.
 */
SalidaModo salida_get_modo(void);

/**
 * @brief Registra un callback opcional para consumir mensajes en modo UX.
 *
 * @param handler Funcion callback; NULL para desregistrar.
 * @param userdata Puntero opaco entregado al callback.
 */
void salida_set_ux_handler(SalidaUxHandler handler, void *userdata);

/**
 * @brief Emite texto formateado por el canal de salida configurado.
 *
 * En modo CLI escribe en stdout. En modo UX usa el callback registrado, y si no
 * existe callback no imprime en terminal.
 *
 * @param formato Cadena de formato estilo printf.
 * @param ... Argumentos variables del formato.
 * @return Cantidad de caracteres producidos o negativa en error.
 */
int salida_printf(const char *formato, ...);

/**
 * @brief Variante con va_list para reutilizar formateo.
 *
 * @param formato Cadena de formato estilo printf.
 * @param args Lista de argumentos ya inicializada.
 * @return Cantidad de caracteres producidos o negativa en error.
 */
int salida_vprintf(const char *formato, va_list args);

/**
 * @brief Emite un mensaje informativo por el canal de salida.
 *
 * @param formato Cadena de formato estilo printf.
 * @param ... Argumentos variables del formato.
 * @return Cantidad de caracteres producidos o negativa en error.
 */
int salida_infof(const char *formato, ...);

/**
 * @brief Emite un mensaje de error por el canal de salida.
 *
 * @param formato Cadena de formato estilo printf.
 * @param ... Argumentos variables del formato.
 * @return Cantidad de caracteres producidos o negativa en error.
 */
int salida_errorf(const char *formato, ...);

#endif /* SALIDA_H */
