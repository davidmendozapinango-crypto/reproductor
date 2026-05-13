#include "salida.h"

#include <stdio.h>
#include <string.h>

#define SALIDA_BUFFER_MAX 1024

static SalidaModo g_modo = SALIDA_MODO_CLI;
static SalidaUxHandler g_ux_handler = NULL;
static void *g_ux_userdata = NULL;

static int salida_emitir_nivel(SalidaNivel nivel, const char *formato, va_list args)
{
    va_list args_copy;
    char buffer[SALIDA_BUFFER_MAX];
    int resultado;

    if (formato == NULL)
    {
        return -1;
    }

    if (g_modo == SALIDA_MODO_CLI)
    {
        if (nivel == SALIDA_NIVEL_ERROR)
        {
            return vfprintf(stderr, formato, args);
        }
        return vprintf(formato, args);
    }

    va_copy(args_copy, args);
    resultado = vsnprintf(buffer, sizeof(buffer), formato, args_copy);
    va_end(args_copy);

    if (resultado < 0)
    {
        return resultado;
    }

    if (g_ux_handler != NULL)
    {
        buffer[sizeof(buffer) - 1] = '\0';
        g_ux_handler(nivel, buffer, g_ux_userdata);
    }

    return resultado;
}

void salida_set_modo(SalidaModo modo)
{
    g_modo = modo;
}

SalidaModo salida_get_modo(void)
{
    return g_modo;
}

void salida_set_ux_handler(SalidaUxHandler handler, void *userdata)
{
    g_ux_handler = handler;
    g_ux_userdata = userdata;
}

int salida_vprintf(const char *formato, va_list args)
{
    return salida_emitir_nivel(SALIDA_NIVEL_INFO, formato, args);
}

int salida_printf(const char *formato, ...)
{
    int resultado;
    va_list args;

    va_start(args, formato);
    resultado = salida_vprintf(formato, args);
    va_end(args);

    return resultado;
}

int salida_infof(const char *formato, ...)
{
    int resultado;
    va_list args;

    va_start(args, formato);
    resultado = salida_emitir_nivel(SALIDA_NIVEL_INFO, formato, args);
    va_end(args);

    return resultado;
}

int salida_errorf(const char *formato, ...)
{
    int resultado;
    va_list args;

    va_start(args, formato);
    resultado = salida_emitir_nivel(SALIDA_NIVEL_ERROR, formato, args);
    va_end(args);

    return resultado;
}
