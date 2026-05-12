#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/cancion.h"

static int contar_nombre_en_espera(const ListaDoble *lista, const char *nombre)
{
    const Cancion *actual;
    int total;

    if (lista == NULL || nombre == NULL)
    {
        return 0;
    }

    total = 0;
    actual = lista->cabeza;
    if (actual != NULL)
    {
        actual = actual->sig;
    }

    while (actual != NULL)
    {
        if (strcmp(actual->nombre, nombre) == 0)
        {
            total++;
        }
        actual = actual->sig;
    }

    return total;
}

static void serializar_cola(const ListaDoble *lista, char *buffer, size_t tam_buffer)
{
    const Cancion *actual;

    if (buffer == NULL || tam_buffer == 0)
    {
        return;
    }

    buffer[0] = '\0';
    if (lista == NULL)
    {
        return;
    }

    actual = lista->cabeza;
    while (actual != NULL)
    {
        if (buffer[0] != '\0')
        {
            strncat(buffer, "|", tam_buffer - strlen(buffer) - 1);
        }
        strncat(buffer, actual->nombre, tam_buffer - strlen(buffer) - 1);
        actual = actual->sig;
    }
}

void test_loop() {
    ColeccionMusical coleccion;
    inicializar_coleccion_musical(&coleccion);

    printf("Probando loop activado y desactivado...\n");

    loop(&coleccion);
    assert(coleccion.lista_reproduccion.loop_activado == 1);
    printf("Loop activado correctamente.\n");

    loop(&coleccion);
    assert(coleccion.lista_reproduccion.loop_activado == 0);
    printf("Loop desactivado correctamente.\n");

    printf("Prueba de loop completada exitosamente.\n");
}

void test_clear_queue() {
    ColeccionMusical coleccion;
    inicializar_coleccion_musical(&coleccion);

    // Agregar canciones a la lista de reproduccion
    insertar_cancion_final(&coleccion.lista_reproduccion.canciones, "Cancion 1", 200);
    insertar_cancion_final(&coleccion.lista_reproduccion.canciones, "Cancion 2", 180);
    insertar_cancion_final(&coleccion.lista_reproduccion.canciones, "Cancion 3", 240);

    printf("Probando clear_queue...\n");

    // Verificar que la lista tiene 3 canciones
    assert(coleccion.lista_reproduccion.canciones.tamano == 3);

    // Llamar a clear_queue
    clear_queue(&coleccion);

    // Verificar que solo queda la cancion actual
    assert(coleccion.lista_reproduccion.canciones.tamano == 1);
    assert(coleccion.lista_reproduccion.canciones.cabeza != NULL);
    assert(coleccion.lista_reproduccion.canciones.cabeza->sig == NULL);

    printf("clear_queue paso todas las pruebas.\n");
}

int main(void)
{
    const char *tmp_path = "tests/tmp_catalogo_test.txt";
    const char *tmp_path_sin_salto = "tests/tmp_catalogo_sin_salto.txt";
    const char *ruta_por_defecto = "C:\\Users\\willmendoza\\Documents\\Visual Studio 2022\\repo\\reproductor\\data\\Catalogo.txt";
    FILE *tmp;
    FILE *tmp_sin_salto;
    ColeccionMusical coleccion;
    ColeccionMusical coleccion_load;
    NodoListaReproduccion *lista_actual;
    int canciones_en_listas;
    int rc;

    tmp = fopen(tmp_path, "w");
    if (tmp == NULL)
    {
        perror("No se pudo crear archivo temporal de prueba");
        return 1;
    }

    fprintf(tmp, "catalogo: cancion1 - cancion2 - cancion3\n");
    fprintf(tmp, "lista rock: cancion1 - cancion2\n");
    fprintf(tmp, "lista pop: cancion2 - cancion3\n");
    fclose(tmp);

    tmp_sin_salto = fopen(tmp_path_sin_salto, "w");
    if (tmp_sin_salto == NULL)
    {
        perror("No se pudo crear archivo temporal sin salto de linea");
        return 1;
    }
    fprintf(tmp_sin_salto, "catalogo: cancion1 - cancion2 - cancion3");
    fclose(tmp_sin_salto);

    {
        Cancion *c;

        c = crear_cancion("demo", 0.0f);
        assert(c != NULL);
        free(c);
    }

    rc = cargar_catalogo_y_listas(tmp_path);
    assert(rc == 0);

    rc = crear_coleccion_desde_archivo(tmp_path, &coleccion);
    assert(rc == 0);
    assert(coleccion.catalogo.tamano == 3);
    assert(coleccion.total_listas == 2);

    rc = cargar_coleccion_desde_ruta(&coleccion_load, tmp_path, NULL, 0);
    assert(rc == 0);
    assert(coleccion_load.catalogo.tamano == 3);
    assert(strcmp(coleccion_load.lista_reproduccion.nombre, "lista de reproduccion") == 0);
    liberar_coleccion_musical(&coleccion_load);

    rc = cargar_coleccion_desde_ruta(&coleccion_load, NULL, NULL, 0);
    assert(rc == 0);
    assert(coleccion_load.catalogo.tamano == 5);
    liberar_coleccion_musical(&coleccion_load);

    rc = cargar_coleccion_desde_ruta(&coleccion_load, "tests/no_existe.txt", NULL, 0);
    assert(rc == 0);

    /* Valida funciones de visualizacion y consulta de listas. */
    mostrar_catalogo(&coleccion);
    listar_listas_disponibles(&coleccion);
    rc = listar_canciones_de_lista(&coleccion, "lista rock");
    assert(rc == 0);
    rc = listar_canciones_de_lista(&coleccion, "lista inexistente");
    assert(rc == -1);

    rc = reproducir_nueva_cancion_o_lista(&coleccion, "play \"cancion2\"");
    assert(rc == 0);
    assert(coleccion.lista_reproduccion.canciones.tamano == 1);
    assert(strcmp(coleccion.lista_reproduccion.canciones.cabeza->nombre, "cancion2") == 0);

    rc = reproducir_nueva_cancion_o_lista(&coleccion, "play \"lista rock\"");
    assert(rc == 0);
    assert(coleccion.lista_reproduccion.canciones.tamano == 2);
    assert(strcmp(coleccion.lista_reproduccion.canciones.cabeza->nombre, "cancion1") == 0);

    rc = agregar_a_cola_reproduccion(&coleccion, "queue \"cancion3\"");
    assert(rc == 0);
    assert(coleccion.lista_reproduccion.canciones.tamano == 3);
    assert(strcmp(coleccion.lista_reproduccion.canciones.cabeza->nombre, "cancion1") == 0);
    assert(strcmp(coleccion.lista_reproduccion.canciones.cabeza->sig->nombre, "cancion3") == 0);
    assert(strcmp(coleccion.lista_reproduccion.canciones.cabeza->sig->sig->nombre, "cancion2") == 0);

    rc = agregar_a_cola_reproduccion(&coleccion, "queue \"lista pop\"");
    assert(rc == 0);
    assert(coleccion.lista_reproduccion.canciones.tamano == 5);
    assert(strcmp(coleccion.lista_reproduccion.canciones.cabeza->nombre, "cancion1") == 0);
    assert(strcmp(coleccion.lista_reproduccion.canciones.cabeza->sig->nombre, "cancion2") == 0);
    assert(strcmp(coleccion.lista_reproduccion.canciones.cabeza->sig->sig->nombre, "cancion3") == 0);
    assert(strcmp(coleccion.lista_reproduccion.canciones.cabeza->sig->sig->sig->nombre, "cancion3") == 0);
    assert(strcmp(coleccion.lista_reproduccion.canciones.cabeza->sig->sig->sig->sig->nombre, "cancion2") == 0);

    rc = agregar_a_cola_reproduccion(&coleccion, "queue \"sin coincidencia\"");
    assert(rc == -1);

    /* Regresion: next avanza y actualiza la cola en orden. */
    rc = reproducir_nueva_cancion_o_lista(&coleccion, "play \"cancion1\"");
    assert(rc == 0);
    rc = agregar_a_cola_reproduccion(&coleccion, "queue \"cancion2\"");
    assert(rc == 0);
    assert(strcmp(coleccion.lista_reproduccion.canciones.cabeza->nombre, "cancion1") == 0);

    next(&coleccion);
    assert(coleccion.lista_reproduccion.canciones.cabeza != NULL);
    assert(strcmp(coleccion.lista_reproduccion.canciones.cabeza->nombre, "cancion2") == 0);

    next(&coleccion);
    assert(coleccion.lista_reproduccion.canciones.cabeza == NULL);

    back(&coleccion);
    assert(coleccion.lista_reproduccion.canciones.cabeza != NULL);
    assert(strcmp(coleccion.lista_reproduccion.canciones.cabeza->nombre, "cancion2") == 0);

    back(&coleccion);
    assert(coleccion.lista_reproduccion.canciones.cabeza != NULL);
    assert(strcmp(coleccion.lista_reproduccion.canciones.cabeza->nombre, "cancion1") == 0);

    /* Regresion: shuffle mezcla solo la cola en espera y desactivar no altera orden. */
    rc = reproducir_nueva_cancion_o_lista(&coleccion, "play \"lista rock\"");
    assert(rc == 0);
    rc = agregar_a_cola_reproduccion(&coleccion, "queue \"lista pop\"");
    assert(rc == 0);
    assert(coleccion.lista_reproduccion.canciones.tamano == 4);
    assert(strcmp(coleccion.lista_reproduccion.canciones.cabeza->nombre, "cancion1") == 0);
    assert(coleccion.lista_reproduccion.shuffle_activado == 0);

    shuffle(&coleccion);
    assert(coleccion.lista_reproduccion.shuffle_activado == 1);
    assert(coleccion.lista_reproduccion.canciones.tamano == 4);
    assert(strcmp(coleccion.lista_reproduccion.canciones.cabeza->nombre, "cancion1") == 0);
    assert(contar_nombre_en_espera(&coleccion.lista_reproduccion.canciones, "cancion2") == 2);
    assert(contar_nombre_en_espera(&coleccion.lista_reproduccion.canciones, "cancion3") == 1);

    {
        char orden_con_shuffle[256];
        char orden_sin_shuffle[256];

        serializar_cola(&coleccion.lista_reproduccion.canciones, orden_con_shuffle, sizeof(orden_con_shuffle));
        shuffle(&coleccion);
        assert(coleccion.lista_reproduccion.shuffle_activado == 0);
        serializar_cola(&coleccion.lista_reproduccion.canciones, orden_sin_shuffle, sizeof(orden_sin_shuffle));
        assert(strcmp(orden_con_shuffle, orden_sin_shuffle) == 0);
    }

    mostrar_reproduccion_actual(&coleccion);

    rc = cargar_nueva_lista_en_ejecucion(
        &coleccion,
        tmp_path,
        "new \"lista runtime\": cancion1 - cancion3");
    assert(rc == 0);

    rc = listar_canciones_de_lista(&coleccion, "lista runtime");
    assert(rc == 0);

    rc = cargar_nueva_lista_en_ejecucion(
        &coleccion,
        tmp_path,
        "new \"lista invalida\": cancion999 - cancion1");
    assert(rc == -1);

    {
        FILE *persistido;
        char linea[256];
        int encontrada;

        persistido = fopen(tmp_path, "r");
        assert(persistido != NULL);

        encontrada = 0;
        while (fgets(linea, sizeof(linea), persistido) != NULL)
        {
            if (strstr(linea, "lista runtime: cancion1 - cancion3") != NULL)
            {
                encontrada = 1;
                break;
            }
        }
        fclose(persistido);
        assert(encontrada == 1);
    }

    {
        ColeccionMusical coleccion_sin_salto;
        FILE *persistido_sin_salto;
        char linea[256];
        int encontrada_final;

        rc = crear_coleccion_desde_archivo(tmp_path_sin_salto, &coleccion_sin_salto);
        assert(rc == 0);
        rc = cargar_nueva_lista_en_ejecucion(
            &coleccion_sin_salto,
            tmp_path_sin_salto,
            "new \"lista final\": cancion1 - cancion2");
        assert(rc == 0);

        persistido_sin_salto = fopen(tmp_path_sin_salto, "r");
        assert(persistido_sin_salto != NULL);

        encontrada_final = 0;
        while (fgets(linea, sizeof(linea), persistido_sin_salto) != NULL)
        {
            if (strstr(linea, "lista final: cancion1 - cancion2") != NULL)
            {
                encontrada_final = 1;
                break;
            }
        }
        fclose(persistido_sin_salto);
        assert(encontrada_final == 1);
        liberar_coleccion_musical(&coleccion_sin_salto);
    }

    canciones_en_listas = 0;
    lista_actual = coleccion.listas;
    while (lista_actual != NULL)
    {
        assert(lista_actual->canciones.tamano >= 2);
        canciones_en_listas += lista_actual->canciones.tamano;
        lista_actual = lista_actual->sig;
    }
    assert(coleccion.total_listas == 3);
    assert(canciones_en_listas == 6);
    liberar_coleccion_musical(&coleccion);

    rc = cargar_catalogo_y_listas(
        ruta_por_defecto);
    assert(rc == 0);

    rc = cargar_catalogo_y_listas("tests/no_existe.txt");
    assert(rc == -1);

    rc = crear_coleccion_desde_archivo("tests/no_existe.txt", &coleccion);
    assert(rc == -1);

    remove(tmp_path);
    remove(tmp_path_sin_salto);

    printf("Pruebas de cargar_catalogo_y_listas completadas correctamente.\n");
    return 0;
}
