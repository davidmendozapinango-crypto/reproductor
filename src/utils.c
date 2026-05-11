#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "utils.h"
#include "cancion.h"

static void trim_line_end(char *texto)
{
    size_t largo;

    largo = strlen(texto);
    while (largo > 0 && (texto[largo - 1] == '\n' || texto[largo - 1] == '\r'))
    {
        texto[largo - 1] = '\0';
        largo--;
    }
}

static char *skip_leading_space(char *texto)
{
    while (*texto != '\0' && isspace((unsigned char)*texto))
    {
        texto++;
    }

    return texto;
}

int cargar_coleccion_desde_ruta(
    ColeccionMusical *coleccion,
    const char *ruta_ingresada,
    char *ruta_activa,
    size_t tam_ruta_activa)
{
    const char *ruta_por_defecto;
    const char *ruta_uso;

    ruta_por_defecto = "C:\\Users\\willmendoza\\Documents\\Visual Studio 2022\\repo\\reproductor\\data\\Catalogo.txt";
    if (ruta_ingresada == NULL || ruta_ingresada[0] == '\0')
    {
        ruta_uso = ruta_por_defecto;
    }
    else
    {
        ruta_uso = ruta_ingresada;
    }

    if (crear_coleccion_desde_archivo(ruta_uso, coleccion) != 0)
    {
        if (ruta_uso != ruta_por_defecto)
        {
            printf("No se pudo cargar la ruta indicada. Se intentara con la ruta por defecto.\n");
            ruta_uso = ruta_por_defecto;
            if (crear_coleccion_desde_archivo(ruta_uso, coleccion) != 0)
            {
                return -1;
            }
        }
        else
        {
            return -1;
        }
    }

    if (ruta_activa != NULL && tam_ruta_activa > 0)
    {
        strncpy(ruta_activa, ruta_uso, tam_ruta_activa - 1);
        ruta_activa[tam_ruta_activa - 1] = '\0';
    }

    return 0;
}

void imprimir_bienvenida(void)
{
    printf("=== Proyecto C Profesional ===\n");
}

void menu(void)
{
    char ruta_ingresada[512];
    char comando[256];
    char ruta_actual[512];
    ColeccionMusical coleccion;

    printf("------MENU------\n - play \"nombre\"\n - current\n - catalog\n - lists\n - songs <nombre_lista>\n - new \"lista\": cancion1 - cancion2\n - q para salir\n");

    printf("Ruta completa del archivo (Enter para usar ruta por defecto):\n");
    if (fgets(ruta_ingresada, sizeof(ruta_ingresada), stdin) == NULL)
    {
        printf("Error al leer la ruta desde terminal.\n");
        return;
    }

    trim_line_end(ruta_ingresada);
    if (cargar_coleccion_desde_ruta(&coleccion, ruta_ingresada, ruta_actual, sizeof(ruta_actual)) != 0)
    {
        return;
    }

    printf("Coleccion cargada. Usa play \"nombre\", current, catalog, lists, songs <nombre_lista>, new \"lista\": cancion1 - cancion2 o q para salir.\n");

    while (1)
    {
        char *parametro;

        printf("> ");
        if (fgets(comando, sizeof(comando), stdin) == NULL)
        {
            break;
        }

        trim_line_end(comando);
        parametro = skip_leading_space(comando);

        if (*parametro == '\0')
        {
            continue;
        }

        if (strcmp(parametro, "q") == 0)
        {
            break;
        }
        else if (strncmp(parametro, "play", 4) == 0)
        {
            (void)reproducir_nueva_cancion_o_lista(&coleccion, parametro);
        }
        else if (strcmp(parametro, "current") == 0)
        {
            mostrar_reproduccion_actual(&coleccion);
        }
        else if (strcmp(parametro, "catalog") == 0)
        {
            mostrar_catalogo(&coleccion);
        }
        else if (strcmp(parametro, "lists") == 0 || strcmp(parametro, "list") == 0)
        {
            listar_listas_disponibles(&coleccion);
        }
        else if (strncmp(parametro, "songs", 5) == 0)
        {
            char *nombre_lista;

            nombre_lista = parametro + 5;
            nombre_lista = skip_leading_space(nombre_lista);
            if (*nombre_lista == '\0')
            {
                printf("Uso: songs <nombre_lista>\n");
            }
            else
            {
                (void)listar_canciones_de_lista(&coleccion, nombre_lista);
            }
        }
        else if (strncmp(parametro, "new", 3) == 0)
        {
            (void)cargar_nueva_lista_en_ejecucion(&coleccion, ruta_actual, parametro);
        }
        else if (strcmp(parametro, "load") == 0)
        {
            printf("Ruta completa del archivo (Enter para usar ruta por defecto):\n");
            if (fgets(ruta_ingresada, sizeof(ruta_ingresada), stdin) == NULL)
            {
                printf("Error al leer la ruta desde terminal.\n");
                continue;
            }

            trim_line_end(ruta_ingresada);
            liberar_coleccion_musical(&coleccion);
            if (cargar_coleccion_desde_ruta(&coleccion, ruta_ingresada, ruta_actual, sizeof(ruta_actual)) != 0)
            {
                printf("No se pudo cargar la coleccion solicitada.\n");
                continue;
            }

            printf("Coleccion cargada desde: %s\n", ruta_actual);
        }
        else
        {
            printf("Comando no reconocido. Usa play \"nombre\", current, catalog, lists, songs <nombre_lista>, new \"lista\": cancion1 - cancion2, load o q.\n");
        }
    }

    liberar_coleccion_musical(&coleccion);
}
