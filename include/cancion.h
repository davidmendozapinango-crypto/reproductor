#ifndef CANCION_H
#define CANCION_H

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
    int               value;
    struct nodo_pila *next;
} NodoPila;

typedef struct {
    NodoPila *tope;
} historial_pila;

typedef struct NodoListaReproduccion {
    char nombre[100];
    ListaDoble canciones;
    struct NodoListaReproduccion *sig;
} NodoListaReproduccion;

typedef struct {
    ListaDoble catalogo;
    NodoListaReproduccion *listas;
    NodoListaReproduccion lista_reproduccion;
    int total_listas;
} ColeccionMusical;

Cancion *crear_cancion(const char *nombre_pista, float duracion_pista);
Cancion *crearCancion(const char *nombre_pista, float duracion_pista);
void vaciar_repro(ListaDoble *repro);
void leerArchivo(const char *nombre_archivo);
int cargar_catalogo_y_listas(const char *ruta_archivo);
void inicializar_lista_doble(ListaDoble *lista);
int insertar_cancion_final(ListaDoble *lista, const char *nombre_cancion, float duracion);
int crear_coleccion_desde_archivo(const char *ruta_archivo, ColeccionMusical *coleccion);
int cargar_coleccion_desde_ruta(
    ColeccionMusical *coleccion,
    const char *ruta_ingresada,
    char *ruta_activa,
    size_t tam_ruta_activa);
void liberar_coleccion_musical(ColeccionMusical *coleccion);
void mostrar_catalogo(const ColeccionMusical *coleccion);
void listar_listas_disponibles(const ColeccionMusical *coleccion);
int listar_canciones_de_lista(const ColeccionMusical *coleccion, const char *nombre_lista);
void mostrar_reproduccion_actual(const ColeccionMusical *coleccion);
int reproducir_nueva_cancion_o_lista(ColeccionMusical *coleccion, const char *comando_play);
int cargar_nueva_lista_en_ejecucion(
    ColeccionMusical *coleccion,
    const char *ruta_archivo,
    const char *comando_new);

#endif /* CANCION_H */