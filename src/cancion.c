#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>

#include "cancion.h"
#include "historial.h"
#include "catalogo.h"

static char *avanzar_espacios(char *texto);
static void recortar_derecha(char *texto);
static int insertar_cancion_inicio(ListaDoble *lista, const char *nombre_cancion, float duracion);

/**
 * @brief Busca una lista de reproduccion por nombre dentro de la coleccion.
 *
 * @param coleccion Coleccion que contiene el listado enlazado de listas.
 * @param nombre_lista Nombre exacto de la lista a localizar.
 * @return Puntero a la lista encontrada o NULL si no existe.
 */
static NodoListaReproduccion *buscar_lista(
    const ColeccionMusical *coleccion,
    const char *nombre_lista)
{
    NodoListaReproduccion *actual;

    actual = coleccion->listas;
    while (actual != NULL)
    {
        if (strcmp(actual->nombre, nombre_lista) == 0)
        {
            return actual;
        }
        actual = actual->sig;
    }

    return NULL;
}

/**
 * @brief Persiste una nueva linea de lista musical al final del archivo fuente.
 *
 * @param ruta_archivo Ruta del archivo de datos donde se anexa la linea.
 * @param linea Texto formateado de la nueva lista a almacenar.
 * @return 0 en exito, -1 si ocurre un error de apertura o escritura.
 */
static int persistir_lista_en_archivo(const char *ruta_archivo, const char *linea)
{
    FILE *archivo;
    int necesita_salto_linea;
    long tamano_archivo;

    necesita_salto_linea = 0;
    archivo = fopen(ruta_archivo, "rb");
    if (archivo == NULL)
    {
        printf("Error: no se pudo abrir el archivo para verificar el final de linea.\n");
        return -1;
    }

    if (fseek(archivo, 0, SEEK_END) == 0)
    {
        tamano_archivo = ftell(archivo);
        if (tamano_archivo > 0 && fseek(archivo, -1, SEEK_END) == 0)
        {
            int ultimo_caracter;

            ultimo_caracter = fgetc(archivo);
            if (ultimo_caracter != '\n' && ultimo_caracter != '\r')
            {
                necesita_salto_linea = 1;
            }
        }
    }

    fclose(archivo);

    archivo = fopen(ruta_archivo, "a");
    if (archivo == NULL)
    {
        printf("Error: no se pudo abrir el archivo para guardar la nueva lista.\n");
        return -1;
    }

    if (necesita_salto_linea && fprintf(archivo, "\n") < 0)
    {
        fclose(archivo);
        printf("Error: no se pudo preparar el archivo para anexar la nueva lista.\n");
        return -1;
    }

    if (fprintf(archivo, "%s\n", linea) < 0)
    {
        fclose(archivo);
        printf("Error: no se pudo persistir la nueva lista en el archivo.\n");
        return -1;
    }

    fclose(archivo);
    return 0;
}

/**
 * @brief Inicializa la coleccion musical completa en un estado vacio valido.
 *
 * @param coleccion Estructura de coleccion a inicializar.
 */

void inicializar_coleccion_musical(ColeccionMusical *coleccion)
{
    if (coleccion == NULL)
    {
        return;
    }

    inicializar_lista_doble(&coleccion->catalogo);
    inicializar_lista_doble(&coleccion->lista_reproduccion.canciones);
    strncpy(
        coleccion->lista_reproduccion.nombre,
        "lista de reproduccion",
        sizeof(coleccion->lista_reproduccion.nombre) - 1);
    coleccion->lista_reproduccion.nombre[sizeof(coleccion->lista_reproduccion.nombre) - 1] = '\0';
    coleccion->lista_reproduccion.historial.tope = NULL;
    coleccion->lista_reproduccion.shuffle_activado = 0;
    coleccion->lista_reproduccion.loop_activado = 0;
    coleccion->lista_reproduccion.sig = NULL;
    coleccion->listas = NULL;
    coleccion->total_listas = 0;
}

/**
 * @brief Busca una cancion por nombre dentro del catalogo general.
 *
 * @param catalogo Catalogo donde se realiza la busqueda lineal.
 * @param nombre_cancion Nombre exacto de la cancion a encontrar.
 * @return Puntero constante a la cancion encontrada o NULL si no existe.
 */
static const Cancion *buscar_cancion_en_catalogo(
    const ListaDoble *catalogo,
    const char *nombre_cancion)
{
    const Cancion *actual;

    if (catalogo == NULL || nombre_cancion == NULL)
    {
        return NULL;
    }

    actual = catalogo->cabeza;
    while (actual != NULL)
    {
        if (strcmp(actual->nombre, nombre_cancion) == 0)
        {
            return actual;
        }
        actual = actual->sig;
    }

    return NULL;
}

/**
 * @brief Vacia la lista de reproduccion activa de la coleccion.
 *
 * @param coleccion Coleccion cuyo buffer de reproduccion se limpia.
 */
static void limpiar_lista_reproduccion(ColeccionMusical *coleccion)
{
    if (coleccion == NULL)
    {
        return;
    }

    vaciar_repro(&coleccion->lista_reproduccion.canciones);
}

/**
 * @brief Inserta una cancion al inicio de una lista doble.
 *
 * @param lista Lista destino donde se inserta la cancion.
 * @param nombre_cancion Nombre de la cancion a crear e insertar.
 * @param duracion Duracion asociada a la cancion.
 * @return 0 en exito, -1 si los parametros son invalidos o falta memoria.
 */
static int insertar_cancion_inicio(ListaDoble *lista, const char *nombre_cancion, float duracion)
{
    Cancion *nueva;

    if (lista == NULL || nombre_cancion == NULL)
    {
        return -1;
    }

    nueva = crear_cancion(nombre_cancion, duracion);
    if (nueva == NULL)
    {
        return -1;
    }

    if (lista->cabeza == NULL)
    {
        lista->cabeza = nueva;
        lista->cola = nueva;
    }
    else
    {
        nueva->sig = lista->cabeza;
        lista->cabeza->ant = nueva;
        lista->cabeza = nueva;
    }

    lista->tamano++;
    return 0;
}

/**
 * @brief Extrae el objetivo del comando play aceptando formato con o sin comillas.
 *
 * @param comando_play Comando completo ingresado por el usuario.
 * @param destino Buffer de salida donde se copia el objetivo parseado.
 * @param tam_destino Tamano total del buffer destino.
 * @return 0 en exito, -1 si el comando es invalido o excede el buffer.
 */
static int extraer_objetivo_play(
    const char *comando_play,
    char *destino,
    size_t tam_destino)
{
    const char *cursor;
    const char *fin;
    size_t largo;

    if (comando_play == NULL || destino == NULL || tam_destino == 0)
    {
        return -1;
    }

    cursor = avanzar_espacios((char *)comando_play);
    if (strncmp(cursor, "play", 4) != 0)
    {
        return -1;
    }

    cursor += 4;
    while (*cursor != '\0' && isspace((unsigned char)*cursor))
    {
        cursor++;
    }

    if (*cursor == '"')
    {
        cursor++;
        fin = strchr(cursor, '"');
        if (fin == NULL)
        {
            return -1;
        }
        largo = (size_t)(fin - cursor);
    }
    else
    {
        fin = cursor + strlen(cursor);
        while (fin > cursor && isspace((unsigned char)*(fin - 1)))
        {
            fin--;
        }
        largo = (size_t)(fin - cursor);
    }

    if (largo == 0 || largo >= tam_destino)
    {
        return -1;
    }

    strncpy(destino, cursor, largo);
    destino[largo] = '\0';
    return 0;
}


/**
 * @brief Extrae el objetivo del comando queue aceptando formato con o sin comillas.
 *
 * @param comando_queue Comando completo queue a interpretar.
 * @param destino Buffer donde se guarda el nombre extraido.
 * @param tam_destino Tamano total del buffer destino.
 * @return 0 en exito, -1 si el formato del comando no es valido.
 */
static int extraer_objetivo_queue(
    const char *comando_queue,
    char *destino,
    size_t tam_destino)
{
    const char *cursor;
    const char *fin;
    size_t largo;

    if (comando_queue == NULL || destino == NULL || tam_destino == 0)
    {
        return -1;
    }

    cursor = avanzar_espacios((char *)comando_queue);
    if (strncmp(cursor, "queue", 5) != 0)
    {
        return -1;
    }

    cursor += 5;
    while (*cursor != '\0' && isspace((unsigned char)*cursor))
    {
        cursor++;
    }

    if (*cursor == '"')
    {
        cursor++;
        fin = strchr(cursor, '"');
        if (fin == NULL)
        {
            return -1;
        }
        largo = (size_t)(fin - cursor);
    }
    else
    {
        fin = cursor + strlen(cursor);
        while (fin > cursor && isspace((unsigned char)*(fin - 1)))
        {
            fin--;
        }
        largo = (size_t)(fin - cursor);
    }

    if (largo == 0 || largo >= tam_destino)
    {
        return -1;
    }

    memcpy(destino, cursor, largo);
    destino[largo] = '\0';
    return 0;
}

/**
 * @brief Inserta un bloque de canciones inmediatamente despues de la cancion actual.
 *
 * @param destino Lista de reproduccion destino.
 * @param bloque Lista temporal con canciones a insertar.
 * @return 0 en exito, -1 si los datos de entrada no son validos.
 */
static int insertar_bloque_despues_de_actual(
    ListaDoble *destino,
    ListaDoble *bloque)
{
    Cancion *actual;
    Cancion *despues_actual;

    if (destino == NULL || bloque == NULL || bloque->cabeza == NULL)
    {
        return -1;
    }

    if (destino->cabeza == NULL)
    {
        destino->cabeza = bloque->cabeza;
        destino->cola = bloque->cola;
        destino->tamano = bloque->tamano;
        bloque->cabeza = NULL;
        bloque->cola = NULL;
        bloque->tamano = 0;
        return 0;
    }

    actual = destino->cabeza;
    despues_actual = actual->sig;

    actual->sig = bloque->cabeza;
    bloque->cabeza->ant = actual;

    if (despues_actual != NULL)
    {
        bloque->cola->sig = despues_actual;
        despues_actual->ant = bloque->cola;
    }
    else
    {
        destino->cola = bloque->cola;
    }

    destino->tamano += bloque->tamano;
    bloque->cabeza = NULL;
    bloque->cola = NULL;
    bloque->tamano = 0;
    return 0;
}

/**
 * @brief Carga una lista guardada dentro de la cola de reproduccion activa.
 *
 * @param coleccion Coleccion donde se actualiza la reproduccion actual.
 * @param lista_origen Lista origen desde la cual se copian las canciones.
 * @return 0 en exito, -1 si ocurre error de parametros o memoria.
 */
static int cargar_lista_en_reproduccion(
    ColeccionMusical *coleccion,
    const NodoListaReproduccion *lista_origen)
{
    const Cancion *actual;

    if (coleccion == NULL || lista_origen == NULL)
    {
        return -1;
    }

    limpiar_lista_reproduccion(coleccion);
    coleccion->lista_reproduccion.shuffle_activado = 0;
    actual = lista_origen->canciones.cabeza;
    while (actual != NULL)
    {
        if (insertar_cancion_final(
                &coleccion->lista_reproduccion.canciones,
                actual->nombre,
                actual->duracion) != 0)
        {
            limpiar_lista_reproduccion(coleccion);
            return -1;
        }

        actual = actual->sig;
    }

    return 0;
}

/**
 * @brief Crea dinamicamente una cancion con nombre y duracion dados.
 *
 * @param nombre_pista Nombre de la pista a almacenar.
 * @param duracion_pista Duracion de la pista en unidades de tiempo del sistema.
 * @return Puntero a la cancion creada o NULL si no hay memoria.
 */
Cancion *crear_cancion(const char *nombre_pista, float duracion_pista)
{
    Cancion *nueva;

    nueva = (Cancion *)malloc(sizeof(Cancion));
    if (nueva == NULL)
    {
        printf("Error: No hay memoria suficiente para crear la cancion.\n");
        return NULL;
    }

    strncpy(nueva->nombre, nombre_pista, sizeof(nueva->nombre) - 1);
    nueva->nombre[sizeof(nueva->nombre) - 1] = '\0';
    nueva->duracion = duracion_pista;
    nueva->sig = NULL;
    nueva->ant = NULL;

    return nueva;
}

/**
 * @brief Libera todos los nodos de una lista doble de canciones.
 *
 * @param repro Lista de reproduccion/catalogo a limpiar completamente.
 */
void vaciar_repro(ListaDoble *repro)
{
    Cancion *actual;
    Cancion *siguiente;

    if (repro == NULL || repro->cabeza == NULL)
    {
        return;
    }

    actual = repro->cabeza;
    while (actual != NULL)
    {
        siguiente = actual->sig;
        free(actual);
        actual = siguiente;
    }

    repro->cola = NULL;
    repro->cabeza = NULL;
    repro->tamano = 0;
}

/**
 * @brief Envoltura historica para cargar y mostrar resumen de un archivo.
 *
 * @param nombre_archivo Ruta del archivo a procesar.
 */
void leerArchivo(const char *nombre_archivo)
{
    (void)cargar_catalogo_y_listas(nombre_archivo);
}

/**
 * @brief Carga una coleccion temporal desde archivo y muestra metricas de resumen.
 *
 * @param ruta_archivo Ruta del archivo de entrada.
 * @return 0 en exito, -1 si la carga falla.
 */
int cargar_catalogo_y_listas(const char *ruta_archivo)
{
    ColeccionMusical coleccion;
    NodoListaReproduccion *lista_actual;
    int total_canciones_listas;

    if (crear_coleccion_desde_archivo(ruta_archivo, &coleccion) != 0)
    {
        return -1;
    }

    total_canciones_listas = 0;
    lista_actual = coleccion.listas;
    while (lista_actual != NULL)
    {
        total_canciones_listas += lista_actual->canciones.tamano;
        lista_actual = lista_actual->sig;
    }

    printf("Carga completada desde: %s\n", ruta_archivo);
    printf("- Canciones en catalogo: %d\n", coleccion.catalogo.tamano);
    printf("- Listas detectadas: %d\n", coleccion.total_listas);
    printf("- Canciones dentro de listas: %d\n", total_canciones_listas);

    liberar_coleccion_musical(&coleccion);

    return 0;
}

/**
 * @brief Avanza el puntero hasta el primer caracter no blanco.
 *
 * @param texto Cadena mutable sobre la cual se posiciona el cursor.
 * @return Puntero al primer caracter util de la cadena.
 */
static char *avanzar_espacios(char *texto)
{
    while (*texto != '\0' && isspace((unsigned char)*texto))
    {
        texto++;
    }

    return texto;
}

/**
 * @brief Elimina espacios en blanco al final de una cadena.
 *
 * @param texto Cadena mutable a recortar por el extremo derecho.
 */
static void recortar_derecha(char *texto)
{
    size_t largo;

    largo = strlen(texto);
    while (largo > 0 && isspace((unsigned char)texto[largo - 1]))
    {
        texto[largo - 1] = '\0';
        largo--;
    }
}

/**
 * @brief Copia una cadena en minusculas para normalizar comparaciones.
 *
 * @param origen Texto de entrada original.
 * @param destino Buffer de salida en minusculas.
 * @param tam_destino Tamano total del buffer destino.
 */
static void normalizar_minusculas(const char *origen, char *destino, size_t tam_destino)
{
    size_t i;

    if (tam_destino == 0)
    {
        return;
    }

    i = 0;
    while (origen[i] != '\0' && i < tam_destino - 1)
    {
        destino[i] = (char)tolower((unsigned char)origen[i]);
        i++;
    }
    destino[i] = '\0';
}

/**
 * @brief Inicializa una lista doble en estado vacio.
 *
 * @param lista Lista a inicializar.
 */
void inicializar_lista_doble(ListaDoble *lista)
{
    if (lista == NULL)
    {
        return;
    }

    lista->cabeza = NULL;
    lista->cola = NULL;
    lista->tamano = 0;
}

/**
 * @brief Inserta una cancion al final de una lista doble.
 *
 * @param lista Lista destino.
 * @param nombre_cancion Nombre de la cancion a insertar.
 * @param duracion Duracion de la cancion a insertar.
 * @return 0 en exito, -1 si hay error de parametros o memoria.
 */
int insertar_cancion_final(ListaDoble *lista, const char *nombre_cancion, float duracion)
{
    Cancion *nueva;

    if (lista == NULL || nombre_cancion == NULL)
    {
        return -1;
    }

    nueva = crear_cancion(nombre_cancion, duracion);
    if (nueva == NULL)
    {
        return -1;
    }

    if (lista->cola == NULL)
    {
        lista->cabeza = nueva;
        lista->cola = nueva;
    }
    else
    {
        nueva->ant = lista->cola;
        lista->cola->sig = nueva;
        lista->cola = nueva;
    }

    lista->tamano++;
    return 0;
}

/**
 * @brief Alias de compatibilidad para crear_cancion.
 *
 * @param nombre_pista Nombre de la pista.
 * @param duracion_pista Duracion de la pista.
 * @return Puntero a la cancion creada o NULL si falla la reserva.
 */
Cancion *crearCancion(const char *nombre_pista, float duracion_pista)
{
    return crear_cancion(nombre_pista, duracion_pista);
}

/**
 * @brief Obtiene una lista existente por nombre o la crea si no existe.
 *
 * @param coleccion Coleccion donde buscar/crear la lista.
 * @param nombre_lista Nombre de la lista objetivo.
 * @return Puntero a la lista encontrada/creada o NULL si falla la memoria.
 */
static NodoListaReproduccion *obtener_o_crear_lista(
    ColeccionMusical *coleccion,
    const char *nombre_lista)
{
    NodoListaReproduccion *actual;
    NodoListaReproduccion *nueva;

    actual = coleccion->listas;
    while (actual != NULL)
    {
        if (strcmp(actual->nombre, nombre_lista) == 0)
        {
            return actual;
        }
        actual = actual->sig;
    }

    nueva = (NodoListaReproduccion *)malloc(sizeof(NodoListaReproduccion));
    if (nueva == NULL)
    {
        return NULL;
    }

    strncpy(nueva->nombre, nombre_lista, sizeof(nueva->nombre) - 1);
    nueva->nombre[sizeof(nueva->nombre) - 1] = '\0';
    inicializar_lista_doble(&nueva->canciones);
    nueva->historial.tope = NULL;
    nueva->shuffle_activado = 0;
    nueva->loop_activado = 0;
    nueva->sig = coleccion->listas;
    coleccion->listas = nueva;
    coleccion->total_listas++;

    return nueva;
}

/**
 * @brief Construye una coleccion musical parseando el archivo de datos.
 *
 * @param ruta_archivo Ruta del archivo de catalogo y listas.
 * @param coleccion Coleccion destino que se inicializa y llena.
 * @return 0 en exito, -1 ante errores de lectura o memoria.
 */
int crear_coleccion_desde_archivo(const char *ruta_archivo, ColeccionMusical *coleccion)
{
    FILE *archivo;
    char linea[1024];

    if (ruta_archivo == NULL || coleccion == NULL)
    {
        return -1;
    }

    inicializar_coleccion_musical(coleccion);

    archivo = fopen(ruta_archivo, "r");
    if (archivo == NULL)
    {
        printf("Error: no se pudo abrir el archivo en la ruta indicada.\n");
        return -1;
    }

    while (fgets(linea, sizeof(linea), archivo) != NULL)
    {
        char *separador;
        char *nombre_bloque;
        char *contenido;
        char nombre_normalizado[128];
        ListaDoble *lista_objetivo;
        char *token;

        nombre_bloque = avanzar_espacios(linea);
        if (*nombre_bloque == '\0' || *nombre_bloque == '\n' || *nombre_bloque == '\r')
        {
            continue;
        }

        separador = strchr(nombre_bloque, ':');
        if (separador == NULL)
        {
            continue;
        }

        *separador = '\0';
        recortar_derecha(nombre_bloque);
        contenido = avanzar_espacios(separador + 1);

        normalizar_minusculas(nombre_bloque, nombre_normalizado, sizeof(nombre_normalizado));
        if (strstr(nombre_normalizado, "catalogo") != NULL || strstr(nombre_normalizado, "catalog") != NULL)
        {
            lista_objetivo = &coleccion->catalogo;
        }
        else
        {
            NodoListaReproduccion *lista_reproduccion;

            lista_reproduccion = obtener_o_crear_lista(coleccion, nombre_bloque);
            if (lista_reproduccion == NULL)
            {
                fclose(archivo);
                liberar_coleccion_musical(coleccion);
                return -1;
            }
            lista_objetivo = &lista_reproduccion->canciones;
        }

        token = strtok(contenido, "-");
        while (token != NULL)
        {
            char *cancion;

            cancion = avanzar_espacios(token);
            recortar_derecha(cancion);

            if (*cancion != '\0' && insertar_cancion_final(lista_objetivo, cancion, 0.0f) != 0)
            {
                vaciar_repro(lista_objetivo);
                
                return -1;
            }

            token = strtok(NULL, "-");
        }
    }

    fclose(archivo);
    return 0;
}

/**
 * @brief Libera todas las estructuras dinamicas asociadas a una coleccion.
 *
 * @param coleccion Coleccion a destruir y dejar en estado liberado.
 */
void liberar_coleccion_musical(ColeccionMusical *coleccion)
{
    NodoListaReproduccion *actual;

    if (coleccion == NULL)
    {
        return;
    }

    vaciar_repro(&coleccion->catalogo);
    vaciar_repro(&coleccion->lista_reproduccion.canciones);
    limpiar_historial_reproduccion(&coleccion->lista_reproduccion.historial);

    actual = coleccion->listas;
    while (actual != NULL)
    {
        NodoListaReproduccion *siguiente;

        siguiente = actual->sig;
        vaciar_repro(&actual->canciones);
        limpiar_historial_reproduccion(&actual->historial);
        free(actual);
        actual = siguiente;
    }

    coleccion->listas = NULL;
    coleccion->total_listas = 0;
}

/**
 * @brief Muestra la cola de reproduccion indicando cancion actual y pendientes.
 *
 * @param coleccion Coleccion de la cual se imprime la reproduccion activa.
 */
void mostrar_reproduccion_actual(const ColeccionMusical *coleccion)
{
    const Cancion *actual;
    int indice;

    if (coleccion == NULL)
    {
        printf("No hay coleccion cargada.\n");
        return;
    }

    printf("=== REPRODUCCION ACTUAL ===\n");
    if (coleccion->lista_reproduccion.canciones.cabeza == NULL)
    {
        printf("No hay reproduccion activa.\n");
        return;
    }

    actual = coleccion->lista_reproduccion.canciones.cabeza;
    indice = 1;
    while (actual != NULL)
    {
        if (indice == 1)
        {
            printf("Actual: %s\n", actual->nombre);
        }
        else
        {
            printf("En cola: %s\n", actual->nombre);
        }

        actual = actual->sig;
        indice++;
    }
}

/**
 * @brief Ejecuta el comando play para reproducir una cancion o lista.
 *
 * @param coleccion Coleccion sobre la que se actualiza la reproduccion.
 * @param comando_play Comando play completo con el objetivo solicitado.
 * @return 0 en exito, -1 si el objetivo no es valido o no existe.
 */
int reproducir_nueva_cancion_o_lista(ColeccionMusical *coleccion, const char *comando_play)
{
    char objetivo[100];
    NodoListaReproduccion *lista;
    const Cancion *cancion;

    if (coleccion == NULL || comando_play == NULL)
    {
        return -1;
    }

    if (extraer_objetivo_play(comando_play, objetivo, sizeof(objetivo)) != 0)
    {
        printf("Error: El comando 'play' requiere un nombre válido.\n");
        return -1;
    }

    lista = buscar_lista(coleccion, objetivo);
    if (lista != NULL)
    {
        printf("Reproduciendo lista: %s\n", objetivo);
        return cargar_lista_en_reproduccion(coleccion, lista);
    }

    cancion = buscar_cancion_en_catalogo(&coleccion->catalogo, objetivo);
    if (cancion != NULL)
    {
        printf("Reproduciendo canción: %s\n", cancion->nombre);
        limpiar_lista_reproduccion(coleccion);
        coleccion->lista_reproduccion.shuffle_activado = 0;
        return insertar_cancion_final(
            &coleccion->lista_reproduccion.canciones,
            cancion->nombre,
            cancion->duracion);
    }

    printf("Error: No se encontró '%s' en el catálogo o las listas.\n", objetivo);
    return -1;
}

/**
 * @brief Inserta en cola una cancion o una lista completa detras de la actual.
 *
 * @param coleccion Coleccion donde se modifica la cola de reproduccion.
 * @param comando_queue Comando queue con el objetivo a insertar.
 * @return 0 en exito, -1 si hay error de parseo, busqueda o memoria.
 */
int agregar_a_cola_reproduccion(ColeccionMusical *coleccion, const char *comando_queue)
{
    char objetivo[100];
    ListaDoble bloque;
    NodoListaReproduccion *lista;
    const Cancion *cancion;
    const Cancion *actual;

    if (coleccion == NULL || comando_queue == NULL)
    {
        return -1;
    }

    if (extraer_objetivo_queue(comando_queue, objetivo, sizeof(objetivo)) != 0)
    {
        printf("Uso: queue \"nombre\"\n");
        return -1;
    }

    inicializar_lista_doble(&bloque);

    lista = buscar_lista(coleccion, objetivo);
    if (lista != NULL)
    {
        actual = lista->canciones.cabeza;
        while (actual != NULL)
        {
            if (insertar_cancion_final(&bloque, actual->nombre, actual->duracion) != 0)
            {
                vaciar_repro(&bloque);
                return -1;
            }
            actual = actual->sig;
        }
    }
    else
    {
        cancion = buscar_cancion_en_catalogo(&coleccion->catalogo, objetivo);
        if (cancion == NULL)
        {
            printf("No se encontro la cancion o lista '%s'.\n", objetivo);
            return -1;
        }

        if (insertar_cancion_final(&bloque, cancion->nombre, cancion->duracion) != 0)
        {
            return -1;
        }
    }

    if (insertar_bloque_despues_de_actual(&coleccion->lista_reproduccion.canciones, &bloque) != 0)
    {
        vaciar_repro(&bloque);
        return -1;
    }

    printf("Se agrego '%s' a la cola de reproduccion.\n", objetivo);
    mostrar_reproduccion_actual(coleccion);
    return 0;
}

/**
 * @brief Imprime por consola el catalogo musical completo.
 *
 * @param coleccion Coleccion que contiene el catalogo a mostrar.
 */
void mostrar_catalogo(const ColeccionMusical *coleccion)
{
    const Cancion *actual;
    int indice;

    if (coleccion == NULL)
    {
        printf("Catalogo no disponible.\n");
        return;
    }

    printf("=== CATALOGO ===\n");
    if (coleccion->catalogo.cabeza == NULL)
    {
        printf("Catalogo vacio.\n");
        return;
    }

    actual = coleccion->catalogo.cabeza;
    indice = 1;
    while (actual != NULL)
    {
        printf("%d. %s\n", indice, actual->nombre);
        actual = actual->sig;
        indice++;
    }
}

/**
 * @brief Lista todas las listas disponibles con su cantidad de canciones.
 *
 * @param coleccion Coleccion que contiene las listas registradas.
 */
void listar_listas_disponibles(const ColeccionMusical *coleccion)
{
    const NodoListaReproduccion *actual;
    int indice;

    if (coleccion == NULL)
    {
        printf("No hay coleccion cargada.\n");
        return;
    }

    printf("=== LISTAS DISPONIBLES ===\n");
    if (coleccion->listas == NULL)
    {
        printf("No hay listas registradas.\n");
        return;
    }

    actual = coleccion->listas;
    indice = 1;
    while (actual != NULL)
    {
        printf("%d. %s (%d canciones)\n", indice, actual->nombre, actual->canciones.tamano);
        actual = actual->sig;
        indice++;
    }
}

/**
 * @brief Muestra las canciones de una lista especifica por nombre.
 *
 * @param coleccion Coleccion donde buscar la lista.
 * @param nombre_lista Nombre de la lista a consultar.
 * @return 0 si se muestra correctamente, -1 si la lista no existe.
 */
int listar_canciones_de_lista(const ColeccionMusical *coleccion, const char *nombre_lista)
{
    const NodoListaReproduccion *lista;
    const Cancion *cancion;
    int indice;

    if (coleccion == NULL || nombre_lista == NULL)
    {
        return -1;
    }

    lista = coleccion->listas;
    while (lista != NULL)
    {
        if (strcmp(lista->nombre, nombre_lista) == 0)
        {
            break;
        }
        lista = lista->sig;
    }

    if (lista == NULL)
    {
        printf("La lista '%s' no existe.\n", nombre_lista);
        return -1;
    }

    printf("=== CANCIONES DE %s ===\n", lista->nombre);
    if (lista->canciones.cabeza == NULL)
    {
        printf("La lista esta vacia.\n");
        return 0;
    }

    cancion = lista->canciones.cabeza;
    indice = 1;
    while (cancion != NULL)
    {
        printf("%d. %s\n", indice, cancion->nombre);
        cancion = cancion->sig;
        indice++;
    }

    return 0;
}

/**
 * @brief Crea una nueva lista en memoria y la persiste en el archivo activo.
 *
 * @param coleccion Coleccion donde se incorpora la nueva lista.
 * @param ruta_archivo Ruta del archivo donde se anexara la lista.
 * @param comando_new Comando completo new con nombre y canciones.
 * @return 0 en exito, -1 si el formato/comprobaciones fallan.
 */
int cargar_nueva_lista_en_ejecucion(
    ColeccionMusical *coleccion,
    const char *ruta_archivo,
    const char *comando_new)
{
    char buffer_comando[1024];
    char linea_a_guardar[1024];
    char *cursor;
    char *nombre_lista_ini;
    char *nombre_lista_fin;
    char *separador;
    char *contenido;
    char nombre_lista[100];
    NodoListaReproduccion *nueva_lista;
    char *token;
    int canciones_agregadas;

    if (coleccion == NULL || ruta_archivo == NULL || comando_new == NULL)
    {
        return -1;
    }

    strncpy(buffer_comando, comando_new, sizeof(buffer_comando) - 1);
    buffer_comando[sizeof(buffer_comando) - 1] = '\0';

    cursor = avanzar_espacios(buffer_comando);
    if (strncmp(cursor, "new", 3) != 0)
    {
        printf("Error: comando invalido. Use: new \"nombre\": cancion1 - cancion2\n");
        return -1;
    }
    cursor += 3;
    cursor = avanzar_espacios(cursor);

    nombre_lista_ini = cursor;
    if (*nombre_lista_ini == '"')
    {
        nombre_lista_ini++;
        nombre_lista_fin = strchr(nombre_lista_ini, '"');
        if (nombre_lista_fin == NULL)
        {
            printf("Error: faltan comillas de cierre en el nombre de la lista.\n");
            return -1;
        }
        *nombre_lista_fin = '\0';
        separador = strchr(nombre_lista_fin + 1, ':');
    }
    else
    {
        separador = strchr(nombre_lista_ini, ':');
        if (separador != NULL)
        {
            *separador = '\0';
            recortar_derecha(nombre_lista_ini);
        }
    }

    if (separador == NULL)
    {
        printf("Error: formato invalido. Use: new \"nombre\": cancion1 - cancion2\n");
        return -1;
    }

    strncpy(nombre_lista, nombre_lista_ini, sizeof(nombre_lista) - 1);
    nombre_lista[sizeof(nombre_lista) - 1] = '\0';
    recortar_derecha(nombre_lista);
    if (nombre_lista[0] == '\0')
    {
        printf("Error: el nombre de la lista no puede estar vacio.\n");
        return -1;
    }

    if (buscar_lista(coleccion, nombre_lista) != NULL)
    {
        printf("Error: la lista '%s' ya existe.\n", nombre_lista);
        return -1;
    }

    nueva_lista = (NodoListaReproduccion *)malloc(sizeof(NodoListaReproduccion));
    if (nueva_lista == NULL)
    {
        printf("Error: no hay memoria para crear una nueva lista.\n");
        return -1;
    }

    strncpy(nueva_lista->nombre, nombre_lista, sizeof(nueva_lista->nombre) - 1);
    nueva_lista->nombre[sizeof(nueva_lista->nombre) - 1] = '\0';
    inicializar_lista_doble(&nueva_lista->canciones);
    nueva_lista->historial.tope = NULL;
    nueva_lista->shuffle_activado = 0;
    nueva_lista->loop_activado = 0;
    nueva_lista->sig = NULL;

    contenido = avanzar_espacios(separador + 1);
    canciones_agregadas = 0;
    token = strtok(contenido, "-");
    while (token != NULL)
    {
        char *cancion;

        cancion = avanzar_espacios(token);
        recortar_derecha(cancion);

        if (*cancion != '\0')
        {
            if (!cancion_existe_en_catalogo(&coleccion->catalogo, cancion))
            {
                printf("Error: la cancion '%s' no existe en el catalogo.\n", cancion);
                vaciar_repro(&nueva_lista->canciones);
                
                return -1;
            }

            if (insertar_cancion_final(&nueva_lista->canciones, cancion, 0.0f) != 0)
            {
                vaciar_repro(&nueva_lista->canciones);
                
                return -1;
            }
            canciones_agregadas++;
        }

        token = strtok(NULL, "-");
    }

    if (canciones_agregadas == 0)
    {
        printf("Error: la lista debe contener al menos una cancion valida.\n");
        
        return -1;
    }

    snprintf(linea_a_guardar, sizeof(linea_a_guardar), "%s: ", nombre_lista);
    {
        Cancion *actual;
        int primera;

        actual = nueva_lista->canciones.cabeza;
        primera = 1;
        while (actual != NULL)
        {
            if (!primera)
            {
                strncat(linea_a_guardar, " - ", sizeof(linea_a_guardar) - strlen(linea_a_guardar) - 1);
            }
            strncat(
                linea_a_guardar,
                actual->nombre,
                sizeof(linea_a_guardar) - strlen(linea_a_guardar) - 1);
            primera = 0;
            actual = actual->sig;
        }
    }

    if (persistir_lista_en_archivo(ruta_archivo, linea_a_guardar) != 0)
    {
        vaciar_repro(&nueva_lista->canciones);
        
        return -1;
    }

    nueva_lista->sig = coleccion->listas;
    coleccion->listas = nueva_lista;
    coleccion->total_listas++;

    printf("Lista '%s' cargada correctamente en ejecucion.\n", nombre_lista);
    return 0;
}

/**
 * @brief Alterna el modo loop de la lista de reproduccion actual.
 *
 * @param coleccion Coleccion cuyo flag de loop se activa o desactiva.
 */
void loop(ColeccionMusical *coleccion) {
    if (coleccion == NULL) {
        return;
    }

    if (coleccion->lista_reproduccion.loop_activado) {
        coleccion->lista_reproduccion.loop_activado = 0;
        printf("Loop desactivado.\n");
    } else {
        coleccion->lista_reproduccion.loop_activado = 1;
        printf("Loop activado.\n");
    }
}

/**
 * @brief Avanza a la siguiente cancion y registra la actual en el historial.
 *
 * @param coleccion Coleccion que contiene la cola e historial de reproduccion.
 */
void next(ColeccionMusical *coleccion) {
    Cancion *cancion_actual;
    NodoPila *nuevo_nodo;

    if (coleccion == NULL) {
        return;
    }

    if (coleccion->lista_reproduccion.canciones.cabeza == NULL) {
        printf("No hay canciones en la cola de reproduccion.\n");
        return;
    }

    cancion_actual = coleccion->lista_reproduccion.canciones.cabeza;
    nuevo_nodo = (NodoPila *)malloc(sizeof(NodoPila));
    if (nuevo_nodo == NULL) {
        printf("Error: no hay memoria para actualizar el historial.\n");
        return;
    }

    strncpy(nuevo_nodo->nombre, cancion_actual->nombre, sizeof(nuevo_nodo->nombre) - 1);
    nuevo_nodo->nombre[sizeof(nuevo_nodo->nombre) - 1] = '\0';
    nuevo_nodo->duracion = cancion_actual->duracion;
    nuevo_nodo->next = coleccion->lista_reproduccion.historial.tope;
    coleccion->lista_reproduccion.historial.tope = nuevo_nodo;

    coleccion->lista_reproduccion.canciones.cabeza = cancion_actual->sig;
    if (coleccion->lista_reproduccion.canciones.cabeza != NULL) {
        coleccion->lista_reproduccion.canciones.cabeza->ant = NULL;
    } else {
        coleccion->lista_reproduccion.canciones.cola = NULL;
    }

    if (coleccion->lista_reproduccion.canciones.tamano > 0) {
        coleccion->lista_reproduccion.canciones.tamano--;
    }

    free(cancion_actual);

    if (coleccion->lista_reproduccion.canciones.cabeza != NULL) {
        printf("Reproduciendo siguiente cancion: %s\n", coleccion->lista_reproduccion.canciones.cabeza->nombre);
    } else if (coleccion->lista_reproduccion.loop_activado) {
        coleccion->lista_reproduccion.canciones.cabeza = coleccion->lista_reproduccion.canciones.cola;
        printf("Reiniciando lista en modo loop: %s\n", coleccion->lista_reproduccion.canciones.cabeza->nombre);
    } else {
        printf("No hay mas canciones en la cola de reproduccion.\n");
    }
}

/**
 * @brief Regresa a la cancion anterior utilizando el historial de reproduccion.
 *
 * @param coleccion Coleccion donde se restaura la cancion previa.
 */
void back(ColeccionMusical *coleccion)
{
    NodoPila *ultimo;
    int rc;

    if (coleccion == NULL)
    {
        return;
    }

    ultimo = coleccion->lista_reproduccion.historial.tope;
    if (ultimo == NULL)
    {
        printf("No hay canciones en el historial de reproduccion.\n");
        return;
    }

    rc = insertar_cancion_inicio(
        &coleccion->lista_reproduccion.canciones,
        ultimo->nombre,
        ultimo->duracion);
    if (rc != 0)
    {
        printf("Error: no se pudo recuperar la cancion del historial.\n");
        return;
    }

    coleccion->lista_reproduccion.historial.tope = ultimo->next;
    printf("Reproduciendo cancion anterior: %s\n", ultimo->nombre);
    free(ultimo);
}

/**
 * @brief Mezcla aleatoriamente las canciones pendientes despues de la actual.
 *
 * @param coleccion Coleccion cuya cola de reproduccion se mezcla.
 */
void shuffle(ColeccionMusical *coleccion)
{
    ListaDoble *reproduccion;
    Cancion *actual;
    Cancion **nodos;
    int cantidad_en_cola;
    int i;
    static int semilla_inicializada = 0;

    if (coleccion == NULL)
    {
        return;
    }

    if (coleccion->lista_reproduccion.shuffle_activado)
    {
        coleccion->lista_reproduccion.shuffle_activado = 0;
        printf("Shuffle desactivado. Se mantiene el orden actual.\n");
        return;
    }

    reproduccion = &coleccion->lista_reproduccion.canciones;
    if (reproduccion->cabeza == NULL)
    {
        printf("No hay canciones en la cola de reproduccion.\n");
        return;
    }

    if (reproduccion->cabeza->sig == NULL)
    {
        printf("No hay canciones en espera para mezclar.\n");
        return;
    }

    cantidad_en_cola = reproduccion->tamano - 1;
    if (cantidad_en_cola <= 1)
    {
        coleccion->lista_reproduccion.shuffle_activado = 1;
        printf("Shuffle activado. Cola en espera mezclada.\n");
        return;
    }

    nodos = (Cancion **)malloc((size_t)cantidad_en_cola * sizeof(Cancion *));
    if (nodos == NULL)
    {
        printf("Error: no hay memoria para aplicar shuffle.\n");
        return;
    }

    actual = reproduccion->cabeza->sig;
    i = 0;
    while (actual != NULL && i < cantidad_en_cola)
    {
        nodos[i] = actual;
        actual = actual->sig;
        i++;
    }

    if (!semilla_inicializada)
    {
        srand((unsigned int)time(NULL));
        semilla_inicializada = 1;
    }

    for (i = cantidad_en_cola - 1; i > 0; i--)
    {
        int j;
        Cancion *tmp;

        j = rand() % (i + 1);
        tmp = nodos[i];
        nodos[i] = nodos[j];
        nodos[j] = tmp;
    }

    reproduccion->cabeza->sig = nodos[0];
    nodos[0]->ant = reproduccion->cabeza;
    for (i = 0; i < cantidad_en_cola - 1; i++)
    {
        nodos[i]->sig = nodos[i + 1];
        nodos[i + 1]->ant = nodos[i];
    }

    nodos[cantidad_en_cola - 1]->sig = NULL;
    reproduccion->cola = nodos[cantidad_en_cola - 1];
    coleccion->lista_reproduccion.shuffle_activado = 1;
    free(nodos);

    printf("Shuffle activado. Cola en espera mezclada.\n");
}

/**
 * @brief Imprime la cancion actual que se esta reproduciendo.
 *
 * @param coleccion Coleccion con la lista de reproduccion activa.
 */
void reproducir_cancion(ColeccionMusical *coleccion) {
    if (coleccion->lista_reproduccion.canciones.cabeza) {
        printf("Reproduciendo: %s\n", coleccion->lista_reproduccion.canciones.cabeza->nombre);
    } else {
        printf("No hay canciones en la lista de reproducción.\n");
    }
}

/**
 * @brief Elimina todas las canciones pendientes de la cola conservando la actual.
 *
 * @param coleccion Coleccion cuya cola de reproduccion sera limpiada.
 */
void clear_queue(ColeccionMusical *coleccion) {
    if (coleccion == NULL || coleccion->lista_reproduccion.canciones.cabeza == NULL) {
        printf("La lista de reproduccion ya esta vacia.\n");
        return;
    }

    Cancion *actual = coleccion->lista_reproduccion.canciones.cabeza->sig;
    Cancion *temp;

    while (actual != NULL) {
        temp = actual;
        actual = actual->sig;
        free(temp);
    }

    coleccion->lista_reproduccion.canciones.cola = coleccion->lista_reproduccion.canciones.cabeza;
    coleccion->lista_reproduccion.canciones.cabeza->sig = NULL;
    coleccion->lista_reproduccion.canciones.tamano = 1;

    printf("La lista de reproduccion ha sido limpiada, manteniendo la cancion actual.\n");
}
