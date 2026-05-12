#ifndef CANCION_H
#define CANCION_H

#include <stddef.h>

/* Defino una cancion como un nodo de lista doblemente enlazada. */
typedef struct Cancion {
    char nombre[100];
    struct Cancion *sig;
    struct Cancion *ant;
    float duracion;
} Cancion;

typedef struct ListaDoble {
    Cancion *cabeza;
    Cancion *cola;
    int tamano;
} ListaDoble;

typedef struct nodo_pila {
    char nombre[100];
    float duracion;
    struct nodo_pila *next;
} NodoPila;

typedef struct {
    NodoPila *tope;
} historial_pila;

typedef struct NodoListaReproduccion {
    char nombre[100];
    ListaDoble canciones;
    historial_pila historial;
    int shuffle_activado;
    int loop_activado;
    struct NodoListaReproduccion *sig;
} NodoListaReproduccion;

typedef struct {
    ListaDoble catalogo;
    NodoListaReproduccion *listas;
    NodoListaReproduccion lista_reproduccion;
    int total_listas;
} ColeccionMusical;

/**
 * @brief Crea una nueva cancion reservando memoria dinamica.
 * @param nombre_pista Nombre de la pista a crear.
 * @param duracion_pista Duracion asociada a la pista.
 * @return Puntero a la cancion creada o NULL si falla la reserva.
 */
Cancion *crear_cancion(const char *nombre_pista, float duracion_pista);

/**
 * @brief Alias de compatibilidad para crear una cancion.
 * @param nombre_pista Nombre de la pista.
 * @param duracion_pista Duracion de la pista.
 * @return Puntero a la cancion creada o NULL si falla.
 */
Cancion *crearCancion(const char *nombre_pista, float duracion_pista);

/**
 * @brief Libera todos los nodos de una lista doble de canciones.
 * @param repro Lista que se desea vaciar por completo.
 */
void vaciar_repro(ListaDoble *repro);

/**
 * @brief Carga datos desde un archivo y muestra resumen de carga.
 * @param nombre_archivo Ruta del archivo a procesar.
 */
void leerArchivo(const char *nombre_archivo);

/**
 * @brief Carga catalogo y listas desde archivo para validacion rapida.
 * @param ruta_archivo Ruta del archivo de entrada.
 * @return 0 si la carga fue exitosa, -1 en caso de error.
 */
int cargar_catalogo_y_listas(const char *ruta_archivo);

/**
 * @brief Inicializa una lista doble en estado vacio.
 * @param lista Lista a inicializar.
 */
void inicializar_lista_doble(ListaDoble *lista);

/**
 * @brief Inserta una cancion al final de una lista doble.
 * @param lista Lista destino.
 * @param nombre_cancion Nombre de la cancion a insertar.
 * @param duracion Duracion de la cancion.
 * @return 0 en exito, -1 si ocurre error de parametros o memoria.
 */
int insertar_cancion_final(ListaDoble *lista, const char *nombre_cancion, float duracion);

/**
 * @brief Construye una coleccion completa a partir de un archivo de datos.
 * @param ruta_archivo Ruta del archivo con catalogo y listas.
 * @param coleccion Estructura destino a rellenar.
 * @return 0 en exito, -1 si falla la lectura o parseo.
 */
int crear_coleccion_desde_archivo(const char *ruta_archivo, ColeccionMusical *coleccion);

/**
 * @brief Carga la coleccion desde ruta indicada o ruta por defecto.
 * @param coleccion Coleccion destino.
 * @param ruta_ingresada Ruta proporcionada por el usuario.
 * @param ruta_activa Buffer opcional para devolver la ruta efectiva.
 * @param tam_ruta_activa Tamano del buffer ruta_activa.
 * @return 0 en exito, -1 si no se pudo cargar la coleccion.
 */
int cargar_coleccion_desde_ruta(
    ColeccionMusical *coleccion,
    const char *ruta_ingresada,
    char *ruta_activa,
    size_t tam_ruta_activa);

/**
 * @brief Libera toda la memoria asociada a una coleccion musical.
 * @param coleccion Coleccion a destruir.
 */
void liberar_coleccion_musical(ColeccionMusical *coleccion);

/**
 * @brief Imprime el catalogo de canciones cargado.
 * @param coleccion Coleccion que contiene el catalogo.
 */
void mostrar_catalogo(const ColeccionMusical *coleccion);

/**
 * @brief Imprime todas las listas disponibles en la coleccion.
 * @param coleccion Coleccion que contiene las listas.
 */
void listar_listas_disponibles(const ColeccionMusical *coleccion);

/**
 * @brief Muestra las canciones de una lista en particular.
 * @param coleccion Coleccion donde buscar la lista.
 * @param nombre_lista Nombre de la lista a mostrar.
 * @return 0 si la lista existe, -1 si no existe.
 */
int listar_canciones_de_lista(const ColeccionMusical *coleccion, const char *nombre_lista);

/**
 * @brief Muestra la reproduccion actual y la cola pendiente.
 * @param coleccion Coleccion con estado de reproduccion.
 */
void mostrar_reproduccion_actual(const ColeccionMusical *coleccion);

/**
 * @brief Procesa un comando play para reproducir cancion o lista.
 * @param coleccion Coleccion sobre la cual se aplica la accion.
 * @param comando_play Comando completo play.
 * @return 0 en exito, -1 si el objetivo no es valido.
 */
int reproducir_nueva_cancion_o_lista(ColeccionMusical *coleccion, const char *comando_play);

/**
 * @brief Reproduce e imprime la cancion actual.
 * @param coleccion Coleccion con la cola de reproduccion activa.
 */
void reproducir_cancion(ColeccionMusical *coleccion);

/**
 * @brief Inserta en cola una cancion o lista usando comando queue.
 * @param coleccion Coleccion donde se modifica la cola.
 * @param comando_queue Comando queue con objetivo.
 * @return 0 en exito, -1 en error.
 */
int agregar_a_cola_reproduccion(ColeccionMusical *coleccion, const char *comando_queue);

/**
 * @brief Crea una lista nueva en ejecucion y la persiste en archivo.
 * @param coleccion Coleccion donde se agrega la lista.
 * @param ruta_archivo Ruta del archivo a actualizar.
 * @param comando_new Comando new completo.
 * @return 0 en exito, -1 si hay error de formato o validacion.
 */
int cargar_nueva_lista_en_ejecucion(
    ColeccionMusical *coleccion,
    const char *ruta_archivo,
    const char *comando_new);

/**
 * @brief Inicializa todos los campos de una coleccion musical.
 * @param coleccion Coleccion a inicializar.
 */
void inicializar_coleccion_musical(ColeccionMusical *coleccion);

/**
 * @brief Avanza a la siguiente cancion en la cola.
 * @param coleccion Coleccion cuyo estado de reproduccion se actualiza.
 */
void next(ColeccionMusical *coleccion);

/**
 * @brief Regresa a la cancion anterior desde el historial.
 * @param coleccion Coleccion cuyo historial se consulta.
 */
void back(ColeccionMusical *coleccion);

/**
 * @brief Activa o desactiva la mezcla aleatoria de la cola pendiente.
 * @param coleccion Coleccion cuya cola se mezcla.
 */
void shuffle(ColeccionMusical *coleccion);

/**
 * @brief Muestra la cancion actual en reproduccion.
 * @param coleccion Coleccion con estado de reproduccion.
 */
void mostrar_cancion_actual(const ColeccionMusical *coleccion);

/**
 * @brief Agrega una cancion ya creada al final de la cola.
 * @param coleccion Coleccion cuyo estado se modifica.
 * @param cancion Nodo de cancion a anexar.
 * @return 0 en exito, -1 si el parametro cancion es invalido.
 */
int agregar_a_cola(ColeccionMusical *coleccion, Cancion *cancion);

/**
 * @brief Activa o desactiva el modo de repeticion continua.
 * @param coleccion Coleccion cuyo flag loop se actualiza.
 */
void loop(ColeccionMusical *coleccion);

/**
 * @brief Limpia la cola pendiente conservando la cancion actual.
 * @param coleccion Coleccion cuya cola de reproduccion se depura.
 */
void clear_queue(ColeccionMusical *coleccion);

#endif /* CANCION_H */
