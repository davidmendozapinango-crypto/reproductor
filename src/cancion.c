#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "cancion.h"

static char *avanzar_espacios(char *texto);
static void recortar_derecha(char *texto);

static int cancion_existe_en_catalogo(const ListaDoble *catalogo, const char *nombre_cancion)
{
    const Cancion *actual;

    actual = catalogo->cabeza;
    while (actual != NULL)
    {
        if (strcmp(actual->nombre, nombre_cancion) == 0)
        {
            return 1;
        }
        actual = actual->sig;
    }

    return 0;
}

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

static void inicializar_coleccion_musical(ColeccionMusical *coleccion)
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
    coleccion->lista_reproduccion.sig = NULL;
    coleccion->listas = NULL;
    coleccion->total_listas = 0;
}

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

static void limpiar_lista_reproduccion(ColeccionMusical *coleccion)
{
    if (coleccion == NULL)
    {
        return;
    }

    vaciar_repro(&coleccion->lista_reproduccion.canciones);
}

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

    memcpy(destino, cursor, largo);
    destino[largo] = '\0';
    return 0;
}

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

void leerArchivo(const char *nombre_archivo)
{
    (void)cargar_catalogo_y_listas(nombre_archivo);
}

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

static char *avanzar_espacios(char *texto)
{
    while (*texto != '\0' && isspace((unsigned char)*texto))
    {
        texto++;
    }

    return texto;
}

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

Cancion *crearCancion(const char *nombre_pista, float duracion_pista)
{
    return crear_cancion(nombre_pista, duracion_pista);
}

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
    nueva->sig = coleccion->listas;
    coleccion->listas = nueva;
    coleccion->total_listas++;

    return nueva;
}

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
                fclose(archivo);
                liberar_coleccion_musical(coleccion);
                return -1;
            }

            token = strtok(NULL, "-");
        }
    }

    fclose(archivo);
    return 0;
}

void liberar_coleccion_musical(ColeccionMusical *coleccion)
{
    NodoListaReproduccion *actual;

    if (coleccion == NULL)
    {
        return;
    }

    vaciar_repro(&coleccion->catalogo);
    vaciar_repro(&coleccion->lista_reproduccion.canciones);

    actual = coleccion->listas;
    while (actual != NULL)
    {
        NodoListaReproduccion *siguiente;

        siguiente = actual->sig;
        vaciar_repro(&actual->canciones);
        free(actual);
        actual = siguiente;
    }

    coleccion->listas = NULL;
    coleccion->total_listas = 0;
}

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
        printf("Uso: play \"nombre\"\n");
        return -1;
    }

    lista = buscar_lista(coleccion, objetivo);
    if (lista != NULL)
    {
        if (cargar_lista_en_reproduccion(coleccion, lista) != 0)
        {
            return -1;
        }

        printf("Reproduciendo lista: %s\n", lista->nombre);
        mostrar_reproduccion_actual(coleccion);
        return 0;
    }

    cancion = buscar_cancion_en_catalogo(&coleccion->catalogo, objetivo);
    if (cancion == NULL)
    {
        printf("No se encontro la cancion o lista '%s'.\n", objetivo);
        return -1;
    }

    limpiar_lista_reproduccion(coleccion);
    if (insertar_cancion_final(
            &coleccion->lista_reproduccion.canciones,
            cancion->nombre,
            cancion->duracion) != 0)
    {
        return -1;
    }

    printf("Reproduciendo cancion: %s\n", cancion->nombre);
    mostrar_reproduccion_actual(coleccion);
    return 0;
}

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
                free(nueva_lista);
                return -1;
            }

            if (insertar_cancion_final(&nueva_lista->canciones, cancion, 0.0f) != 0)
            {
                vaciar_repro(&nueva_lista->canciones);
                free(nueva_lista);
                return -1;
            }
            canciones_agregadas++;
        }

        token = strtok(NULL, "-");
    }

    if (canciones_agregadas == 0)
    {
        printf("Error: la lista debe contener al menos una cancion valida.\n");
        free(nueva_lista);
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
        free(nueva_lista);
        return -1;
    }

    nueva_lista->sig = coleccion->listas;
    coleccion->listas = nueva_lista;
    coleccion->total_listas++;

    printf("Lista '%s' cargada correctamente en ejecucion.\n", nombre_lista);
    return 0;
}
