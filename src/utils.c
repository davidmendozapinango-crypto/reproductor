#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "utils.h"
#include "cancion.h"

/**
 * @brief Elimina saltos de linea y retorno de carro al final de una cadena.
 *
 * Recorre la cadena desde el final y reemplaza '\n' o '\r' por terminador
 * nulo hasta encontrar un caracter valido de contenido.
 *
 * @param texto Cadena mutable que se normaliza en sitio.
 */
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

/**
 * @brief Retorna un puntero al primer caracter no blanco de una cadena.
 *
 * Omite espacios y tabulaciones iniciales para facilitar el parseo de
 * comandos escritos por el usuario en consola.
 *
 * @param texto Cadena de entrada a analizar.
 * @return Puntero al inicio del contenido util dentro de la misma cadena.
 */
static char *skip_leading_space(char *texto)
{
    while (*texto != '\0' && isspace((unsigned char)*texto))
    {
        texto++;
    }

    return texto;
}

/**
 * @brief Carga una coleccion musical desde una ruta explicita o por defecto.
 *
 * Intenta leer la coleccion desde la ruta ingresada. Si falla y la ruta era
 * personalizada, realiza un segundo intento con la ruta por defecto del
 * proyecto. Opcionalmente deja registrada la ruta efectiva usada.
 *
 * @param coleccion Coleccion que se inicializa y rellena con los datos cargados.
 * @param ruta_ingresada Ruta solicitada por el usuario; puede ser NULL o vacia.
 * @param ruta_activa Buffer opcional para devolver la ruta que se uso realmente.
 * @param tam_ruta_activa Tamano total del buffer ruta_activa.
 * @return 0 en exito, -1 si no se pudo cargar ninguna ruta valida.
 */
int cargar_coleccion_desde_ruta(
    ColeccionMusical *coleccion,
    const char *ruta_ingresada,
    char *ruta_activa,
    size_t tam_ruta_activa)
{
    const char *rutas_por_defecto[] = {
        "data/Catalogo.txt",
        "data/catalog.txt",
        "../data/Catalogo.txt",
        "../data/catalog.txt"};
    const int total_rutas_por_defecto = (int)(sizeof(rutas_por_defecto) / sizeof(rutas_por_defecto[0]));
    const char *ruta_uso;
    int i;

    ruta_uso = NULL;
    if (ruta_ingresada != NULL && ruta_ingresada[0] != '\0')
    {
        if (crear_coleccion_desde_archivo(ruta_ingresada, coleccion) == 0)
        {
            ruta_uso = ruta_ingresada;
        }
        else
        {
            printf("No se pudo cargar la ruta indicada. Se intentara con la ruta por defecto.\n");
        }
    }

    if (ruta_uso == NULL)
    {
        for (i = 0; i < total_rutas_por_defecto; i++)
        {
            if (crear_coleccion_desde_archivo(rutas_por_defecto[i], coleccion) == 0)
            {
                ruta_uso = rutas_por_defecto[i];
                break;
            }
        }
    }

    if (ruta_uso == NULL)
    {
        return -1;
    }

    if (ruta_activa != NULL && tam_ruta_activa > 0)
    {
        strncpy(ruta_activa, ruta_uso, tam_ruta_activa - 1);
        ruta_activa[tam_ruta_activa - 1] = '\0';
    }

    return 0;
}

/**
 * @brief Muestra el encabezado de bienvenida del programa.
 */
void imprimir_bienvenida(void)
{
    printf("=== Proyecto C Profesional ===\n");
}

/**
 * @brief Ejecuta el menu interactivo principal en modo consola.
 *
 * Solicita la ruta del archivo de datos, carga la coleccion y procesa el
 * bucle de comandos del usuario hasta recibir la orden de salida.
 */
void menu(void)
{
    char ruta_ingresada[512];
    char comando[256];
    char ruta_actual[512];
    ColeccionMusical coleccion;

    printf("------MENU------\n - play \"nombre\"\n - queue \"nombre\"\n - next\n - back\n - shuffle\n - current\n - catalog\n - lists\n - songs <nombre_lista>\n - new \"lista\": cancion1 - cancion2\n - q para salir\n");

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

    printf("Coleccion cargada. Usa play \"nombre\", queue \"nombre\", next, back, shuffle, current, catalog, lists, songs <nombre_lista>, new \"lista\": cancion1 - cancion2 o q para salir.\n");

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
        else if (strncmp(parametro, "queue", 5) == 0)
        {
            if (strlen(parametro) > 6)
            {
                (void)agregar_a_cola_reproduccion(&coleccion, parametro);
            }
            else
            {
                printf("Error: El comando 'queue' requiere un argumento.\n");
            }
        }
        else if (strcmp(parametro, "current") == 0)
        {
            mostrar_reproduccion_actual(&coleccion);
        }
        else if (strcmp(parametro, "next") == 0)
        {
            next(&coleccion);
        }
        else if (strcmp(parametro, "back") == 0)
        {
            back(&coleccion);
        }
        else if (strcmp(parametro, "shuffle") == 0)
        {
            shuffle(&coleccion);
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
            printf("Comando no reconocido. Usa play \"nombre\", queue \"nombre\", next, back, shuffle, current, catalog, lists, songs <nombre_lista>, new \"lista\": cancion1 - cancion2, load o q.\n");
        }
    }

    liberar_coleccion_musical(&coleccion);
}

/**
 * @brief Muestra un menu numerado simplificado de opciones.
 */
void mostrar_menu() {
    printf("Opciones:\n");
    printf("1. Cargar colección\n");
    printf("2. Reproducir canción\n");
    printf("3. Mostrar canción actual\n");
    printf("4. Agregar a la cola\n");
    printf("5. Siguiente canción\n"); // Added next command
    printf("6. Salir\n");
}

/**
 * @brief Procesa una opcion numerica y ejecuta la accion correspondiente.
 *
 * @param opcion Codigo de opcion seleccionada por el usuario.
 * @param coleccion Coleccion sobre la que se aplica la accion.
 */
void procesar_opcion(int opcion, ColeccionMusical *coleccion) {
    switch (opcion) {
        case 1:
            cargar_coleccion_desde_ruta(coleccion, NULL, NULL, 0);
            break;
        case 2:
            reproducir_cancion(coleccion);
            break;
        case 3:
            mostrar_cancion_actual(coleccion);
            break;
        case 4: {
            Cancion *nueva_cancion = crear_cancion("Nueva Cancion", 3.5);
            if (nueva_cancion) {
                agregar_a_cola(coleccion, nueva_cancion);
            } else {
                printf("Error: No se pudo crear la canción.\n");
            }
            break;
        }
        case 5:
            next(coleccion); // Added next command handling
            break;
        case 6:
            printf("Saliendo...\n");
            break;
        default:
            printf("Opción no válida.\n");
            break;
    }
}

/**
 * @brief Muestra por consola la cancion actual en reproduccion.
 *
 * @param coleccion Coleccion que contiene la lista de reproduccion activa.
 */
void mostrar_cancion_actual(const ColeccionMusical *coleccion) {
    if (coleccion->lista_reproduccion.canciones.cabeza) {
        printf("Canción actual: %s\n", coleccion->lista_reproduccion.canciones.cabeza->nombre);
    } else {
        printf("No hay canción en reproducción.\n");
    }
}

/**
 * @brief Agrega una cancion ya creada al final de la cola de reproduccion.
 *
 * @param coleccion Coleccion cuyo estado de reproduccion se actualiza.
 * @param cancion Nodo de cancion ya reservado en memoria para anexar a la cola.
 * @return 0 en exito, -1 si la cancion es invalida.
 */
int agregar_a_cola(ColeccionMusical *coleccion, Cancion *cancion) {
    if (!cancion) return -1;

    if (!coleccion->lista_reproduccion.canciones.cabeza) {
        coleccion->lista_reproduccion.canciones.cabeza = cancion;
        coleccion->lista_reproduccion.canciones.cola = cancion;
    } else {
        coleccion->lista_reproduccion.canciones.cola->sig = cancion;
        cancion->ant = coleccion->lista_reproduccion.canciones.cola;
        coleccion->lista_reproduccion.canciones.cola = cancion;
    }

    coleccion->lista_reproduccion.canciones.tamano++;
    return 0;
}

/**
 * @brief Despacha comandos de control rapido sobre la reproduccion.
 *
 * Reconoce comandos de navegacion como next/back, modos de reproduccion y
 * limpieza de cola.
 *
 * @param comando Texto del comando a ejecutar.
 * @param coleccion Coleccion sobre la cual se aplica el comando.
 */
void procesar_comando(const char *comando, ColeccionMusical *coleccion) {
    if (strcmp(comando, "next") == 0) {
        next(coleccion);
    } else if (strcmp(comando, "back") == 0) {
        back(coleccion);
    } else if (strcmp(comando, "shuffle") == 0) {
        shuffle(coleccion);
    } else if (strcmp(comando, "loop") == 0) {
        loop(coleccion);
    } else if (strcmp(comando, "clear queue") == 0) {
        clear_queue(coleccion);
    } else {
        printf("Comando no reconocido: %s\n", comando);
    }
}
