/*
 * ui.c
 * Este archivo contiene la implementación de la interfaz de usuario basada en terminal utilizando la biblioteca NCURSES.
 * Proporciona funciones para manejar la entrada del usuario, dibujar paneles y simular la reproducción de canciones.
 */
#if defined(__has_include)
#if __has_include(<ncurses.h>)
#include <ncurses.h>
#elif __has_include(<curses.h>)
#include <curses.h>
#else
#error "No se encontro ncurses.h/curses.h. Instala ncurses o pdcurses."
#endif
#else
#include <ncurses.h>
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cancion.h"
#include "salida.h"
#include "ui.h"

#define CMD_MAX_LEN 256
#define STATUS_MAX_LEN 192
#define PATH_MAX_LEN 512
#define LOG_LINES_MAX 64

typedef struct SimTrackSnapshot SimTrackSnapshot;

typedef struct UiContext
{
    ColeccionMusical coleccion;
    char ruta_actual[PATH_MAX_LEN];
    char comando[CMD_MAX_LEN];
    char estado[STATUS_MAX_LEN];
    char reproduccion_estado[STATUS_MAX_LEN];
    char reproduccion_modo[32];
    char reproduccion_fuente[32];
    char log_lineas[LOG_LINES_MAX][STATUS_MAX_LEN];
    SalidaNivel log_niveles[LOG_LINES_MAX];
    size_t comando_len;
    int log_inicio;
    int log_total;
    int indice_lista;
    int indice_track;
    int foco_panel;
    int log_solo_errores;
    int simulacion_activa;
    int simulacion_omitir_actual;
    int simulacion_total;
    int simulacion_indice;
    int simulacion_ciclo_actual;
    int simulacion_ciclos;
    time_t simulacion_ultimo_tick;
    SimTrackSnapshot *simulacion_inicio;
    SimTrackSnapshot *simulacion_actual;
    SimTrackSnapshot *simulacion_fin;
    SimTrackSnapshot *simulacion_punto_loop;
    int salir;
} UiContext;

struct SimTrackSnapshot
{
    char nombre[100];
    float duracion;
    struct SimTrackSnapshot *sig;
};

static WINDOW *win_artists;
static WINDOW *win_tracks;
static WINDOW *win_queue;
static WINDOW *win_status;
static WINDOW *win_log;
static WINDOW *win_cmd;

static void draw_layout(void);
static void cleanup(void);
static void handle_input(UiContext *ctx);
static void redraw_all(const UiContext *ctx);
static void draw_artists_panel(const UiContext *ctx);
static void draw_tracks_panel(const UiContext *ctx);
static void draw_queue_panel(const UiContext *ctx);
static void draw_status_panel(const UiContext *ctx);
static void draw_log_panel(const UiContext *ctx);
static void draw_command_panel(const UiContext *ctx);
static int total_listas_ui(const UiContext *ctx);
static const ListaDoble *lista_por_indice(const UiContext *ctx, int indice, const char **nombre);
static const Cancion *cancion_por_indice(const ListaDoble *lista, int indice);
static void set_estado(UiContext *ctx, const char *mensaje);
static void set_estado_nivel(UiContext *ctx, SalidaNivel nivel, const char *mensaje);
static void set_reproduccion_estado(UiContext *ctx, const char *mensaje);
static void set_reproduccion_modo(UiContext *ctx, const char *modo);
static void set_reproduccion_fuente(UiContext *ctx, const char *fuente);
static int color_para_modo(const char *modo);
static void liberar_simulacion(UiContext *ctx);
static int agregar_snapshost_simulacion(UiContext *ctx, const char *nombre, float duracion);
static int contar_snapshot_simulacion(const UiContext *ctx);
static void ui_append_log(UiContext *ctx, SalidaNivel nivel, const char *mensaje);
static void ui_salida_handler(SalidaNivel nivel, const char *mensaje, void *userdata);
static void simular_reproduccion_ux(UiContext *ctx, int omitir_actual);
static void avanzar_simulacion_ux(UiContext *ctx);
static void asegurar_simulacion_con_loop(UiContext *ctx);
static void ejecutar_comando(UiContext *ctx, const char *comando);
static void trim_line_end(char *texto);
static char *skip_leading_space(char *texto);
static void draw_hotkey(WINDOW *win, int y, int x, char key, const char *label);
static int procesar_atajo_alt(UiContext *ctx);

void ui_main(void)
{
    UiContext ctx;

    memset(&ctx, 0, sizeof(ctx));
    inicializar_coleccion_musical(&ctx.coleccion);
    strncpy(ctx.ruta_actual,
            "data/Catalogo.txt",
            sizeof(ctx.ruta_actual) - 1);

    if (cargar_coleccion_desde_ruta(&ctx.coleccion, "", ctx.ruta_actual, sizeof(ctx.ruta_actual)) != 0)
    {
        set_estado_nivel(&ctx, SALIDA_NIVEL_ERROR, "No se pudo cargar el catalogo inicial.");
    }
    else
    {
        set_estado(&ctx, "Coleccion cargada. Usa Enter para ejecutar comandos.");
        set_reproduccion_estado(&ctx, "reproduciendo --");
        set_reproduccion_modo(&ctx, "IDLE");
        set_reproduccion_fuente(&ctx, "");
    }

    salida_set_ux_handler(ui_salida_handler, &ctx);

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    start_color();
    use_default_colors();
    init_pair(1, COLOR_YELLOW, -1);
    init_pair(2, COLOR_RED, -1);
    init_pair(3, COLOR_GREEN, -1);
    init_pair(4, COLOR_CYAN, -1);
    init_pair(5, COLOR_WHITE, -1);

    draw_layout();
    handle_input(&ctx);
    liberar_simulacion(&ctx);
    salida_set_ux_handler(NULL, NULL);
    cleanup();
    liberar_coleccion_musical(&ctx.coleccion);
}

static void draw_layout(void)
{
    int rows;
    int cols;
    int artists_w;
    int tracks_w;
    int tracks_h;
    int queue_h;
    int status_h;
    int log_h;
    int cmd_h;
    int main_h;

    getmaxyx(stdscr, rows, cols);
    artists_w = cols / 4;
    tracks_w = cols - artists_w;
    status_h = 3;
    log_h = 5;
    cmd_h = 3;
    main_h = rows - status_h - log_h - cmd_h;

    if (main_h < 8)
    {
        log_h = 3;
        main_h = rows - status_h - log_h - cmd_h;
    }

    tracks_h = (main_h * 2) / 3;
    if (tracks_h < 6)
    {
        tracks_h = main_h / 2;
    }
    queue_h = main_h - tracks_h;

    win_artists = newwin(main_h, artists_w, 0, 0);
    win_tracks = newwin(tracks_h, tracks_w, 0, artists_w);
    win_queue = newwin(queue_h, tracks_w, tracks_h, artists_w);
    win_status = newwin(status_h, cols, main_h, 0);
    win_log = newwin(log_h, cols, main_h + status_h, 0);
    win_cmd = newwin(cmd_h, cols, main_h + status_h + log_h, 0);

    keypad(win_cmd, TRUE);
    wtimeout(win_cmd, 100);
    wbkgd(win_artists, COLOR_PAIR(5));
    wbkgd(win_tracks, COLOR_PAIR(5));
    wbkgd(win_queue, COLOR_PAIR(5));
    wbkgd(win_status, COLOR_PAIR(4));
    wbkgd(win_log, COLOR_PAIR(5));
    wbkgd(win_cmd, COLOR_PAIR(5));
}

static void handle_input(UiContext *ctx)
{
    int ch;

    while (!ctx->salir)
    {
        asegurar_simulacion_con_loop(ctx);
        avanzar_simulacion_ux(ctx);
        redraw_all(ctx);
        ch = wgetch(win_cmd);

        if (ch == ERR)
        {
            continue;
        }

        if (ch == 4)
        {
            ctx->salir = 1;
            continue;
        }

        if (ch == KEY_LEFT)
        {
            ctx->foco_panel = 0;
            continue;
        }

        if (ch == KEY_RIGHT || ch == '\t')
        {
            ctx->foco_panel = 1;
            continue;
        }

        if (ch == KEY_UP)
        {
            if (ctx->foco_panel == 0 && ctx->indice_lista > 0)
            {
                ctx->indice_lista--;
                ctx->indice_track = 0;
            }
            else if (ctx->foco_panel == 1 && ctx->indice_track > 0)
            {
                ctx->indice_track--;
            }
            continue;
        }

        if (ch == KEY_DOWN)
        {
            if (ctx->foco_panel == 0)
            {
                int max_lista;

                max_lista = total_listas_ui(ctx) - 1;
                if (ctx->indice_lista < max_lista)
                {
                    ctx->indice_lista++;
                    ctx->indice_track = 0;
                }
            }
            else
            {
                const ListaDoble *lista;
                int max_track;

                lista = lista_por_indice(ctx, ctx->indice_lista, NULL);
                max_track = (lista != NULL) ? lista->tamano - 1 : -1;
                if (ctx->indice_track < max_track)
                {
                    ctx->indice_track++;
                }
            }
            continue;
        }

        if (ch == 27)
        {
            if (!procesar_atajo_alt(ctx))
            {
                ctx->comando[0] = '\0';
                ctx->comando_len = 0;
                set_estado(ctx, "Entrada de comando limpiada.");
            }
            continue;
        }

        if (ch == KEY_BACKSPACE || ch == 127 || ch == 8)
        {
            if (ctx->comando_len > 0)
            {
                ctx->comando_len--;
                ctx->comando[ctx->comando_len] = '\0';
            }
            continue;
        }

        // Eliminado Ctrl+P (ch == 16) para ejecutar comando, ahora se usa Enter

        if (ch == '\n' || ch == KEY_ENTER || ch == '\r')
        {
            if (ctx->comando_len == 0)
            {
                const ListaDoble *lista;
                const Cancion *track;
                char comando_play[CMD_MAX_LEN];

                lista = lista_por_indice(ctx, ctx->indice_lista, NULL);
                track = cancion_por_indice(lista, ctx->indice_track);
                if (track != NULL)
                {
                    snprintf(comando_play, sizeof(comando_play), "play \"%s\"", track->nombre);
                    strncpy(ctx->comando, comando_play, sizeof(ctx->comando) - 1);
                    ctx->comando[sizeof(ctx->comando) - 1] = '\0';
                    ctx->comando_len = strlen(ctx->comando);
                    set_estado(ctx, "Pista seleccionada. Ejecuta con Enter.");
                }
                else
                {
                    set_estado(ctx, "No hay pista seleccionada para reproducir.");
                }
            }
            else
            {
                ejecutar_comando(ctx, ctx->comando);
                ctx->comando[0] = '\0';
                ctx->comando_len = 0;
            }
            continue;
        }

        if (isprint(ch) && ctx->comando_len < (CMD_MAX_LEN - 1))
        {
            ctx->comando[ctx->comando_len] = (char)ch;
            ctx->comando_len++;
            ctx->comando[ctx->comando_len] = '\0';
        }
    }
}

static void asegurar_simulacion_con_loop(UiContext *ctx)
{
    if (ctx == NULL)
    {
        return;
    }

    if (!ctx->coleccion.lista_reproduccion.loop_activado)
    {
        return;
    }

    if (ctx->coleccion.lista_reproduccion.canciones.cabeza == NULL)
    {
        return;
    }

    if (!ctx->simulacion_activa)
    {
        set_estado(ctx, "Loop activo: iniciando simulacion continua.");
        set_reproduccion_modo(ctx, "LOOP");
        set_reproduccion_fuente(ctx, "LOOP");
        simular_reproduccion_ux(ctx, 0);
    }
}

static int procesar_atajo_alt(UiContext *ctx)
{
    int alt_key;

    wtimeout(win_cmd, 30);
    alt_key = wgetch(win_cmd);
    wtimeout(win_cmd, -1);

    if (alt_key == ERR)
    {
        return 0;
    }

    alt_key = tolower(alt_key);
    if (alt_key == 'b')
    {
        ejecutar_comando(ctx, "back");
        return 1;
    }
    if (alt_key == 'n')
    {
        ejecutar_comando(ctx, "next");
        return 1;
    }
    if (alt_key == 's')
    {
        ejecutar_comando(ctx, "shuffle");
        return 1;
    }
    if (alt_key == 'l')
    {
        ejecutar_comando(ctx, "loop");
        return 1;
    }
    if (alt_key == 'c')
    {
        ejecutar_comando(ctx, "clear_queue");
        return 1;
    }
    if (alt_key == 'e')
    {
        ctx->log_solo_errores = !ctx->log_solo_errores;
        if (ctx->log_solo_errores)
        {
            set_estado(ctx, "Filtro de eventos: solo errores.");
        }
        else
        {
            set_estado(ctx, "Filtro de eventos: todos.");
        }
        return 1;
    }
    if (alt_key == 'q')
    {
        ejecutar_comando(ctx, "quit");
        return 1;
    }

    set_estado(ctx, "Atajo ALT no reconocido.");
    return 1;
}

static void redraw_all(const UiContext *ctx)
{
    draw_artists_panel(ctx);
    draw_tracks_panel(ctx);
    draw_queue_panel(ctx);
    draw_status_panel(ctx);
    draw_log_panel(ctx);
    draw_command_panel(ctx);
}

static void draw_artists_panel(const UiContext *ctx)
{
    int height;
    int cols;
    int text_w;
    int visible;
    int total;
    int i;

    getmaxyx(win_artists, height, cols);
    werase(win_artists);
    box(win_artists, 0, 0);
    wattron(win_artists, COLOR_PAIR(2) | A_BOLD);
    mvwprintw(win_artists, 0, 2, " listas ");
    wattroff(win_artists, COLOR_PAIR(2) | A_BOLD);

    total = total_listas_ui(ctx);
    visible = height - 2;
    text_w = (cols > 4) ? (cols - 4) : 1;
    for (i = 0; i < visible && i < total; i++)
    {
        const char *nombre_lista;

        lista_por_indice(ctx, i, &nombre_lista);
        if (i == ctx->indice_lista)
        {
            wattron(win_artists, COLOR_PAIR(1) | A_BOLD);
        }
        mvwprintw(win_artists, i + 1, 2, "%-*.*s", text_w, text_w, nombre_lista);
        if (i == ctx->indice_lista)
        {
            wattroff(win_artists, COLOR_PAIR(1) | A_BOLD);
        }
    }

    if (ctx->foco_panel == 0)
    {
        wattron(win_artists, COLOR_PAIR(3) | A_BOLD);
        mvwprintw(win_artists, height - 1, 2, "foco");
        wattroff(win_artists, COLOR_PAIR(3) | A_BOLD);
    }

    wrefresh(win_artists);
}

static void draw_tracks_panel(const UiContext *ctx)
{
    int height;
    int cols;
    int name_w;
    int row;
    const ListaDoble *lista;
    const char *nombre_lista;
    const Cancion *actual;
    int indice;

    getmaxyx(win_tracks, height, cols);
    werase(win_tracks);
    box(win_tracks, 0, 0);

    lista = lista_por_indice(ctx, ctx->indice_lista, &nombre_lista);
    wattron(win_tracks, COLOR_PAIR(2) | A_BOLD);
    mvwprintw(win_tracks, 0, 2, " pistas: %s ", nombre_lista);
    wattroff(win_tracks, COLOR_PAIR(2) | A_BOLD);

    row = 1;
    name_w = (cols > 16) ? (cols - 16) : 8;
    indice = 0;
    actual = (lista != NULL) ? lista->cabeza : NULL;
    while (actual != NULL && row < (height - 1))
    {
        if (indice == ctx->indice_track)
        {
            wattron(win_tracks, COLOR_PAIR(1) | A_BOLD);
        }

        mvwprintw(win_tracks,
                  row,
                  2,
                  "%2d  %-*.*s %6.2f",
                  indice + 1,
                  name_w,
                  name_w,
                  actual->nombre,
                  actual->duracion);

        if (indice == ctx->indice_track)
        {
            wattroff(win_tracks, COLOR_PAIR(1) | A_BOLD);
        }

        actual = actual->sig;
        indice++;
        row++;
    }

    if (ctx->foco_panel == 1)
    {
        wattron(win_tracks, COLOR_PAIR(3) | A_BOLD);
        mvwprintw(win_tracks, height - 1, 2, "foco");
        wattroff(win_tracks, COLOR_PAIR(3) | A_BOLD);
    }

    wrefresh(win_tracks);
}

static void draw_queue_panel(const UiContext *ctx)
{
    int height;
    int cols;
    int row;
    int name_w;
    const Cancion *actual;
    const char *nombre_objetivo;
    int aparicion_objetivo;
    int indice;

    getmaxyx(win_queue, height, cols);
    werase(win_queue);
    box(win_queue, 0, 0);
    wattron(win_queue, COLOR_PAIR(2) | A_BOLD);
    mvwprintw(win_queue, 0, 2, " cola actual ");
    wattroff(win_queue, COLOR_PAIR(2) | A_BOLD);

    actual = ctx->coleccion.lista_reproduccion.canciones.cabeza;
    if (actual == NULL)
    {
        mvwprintw(win_queue, 1, 2, "Sin canciones en cola.");
        wrefresh(win_queue);
        return;
    }

    row = 1;
    indice = 0;
    name_w = (cols > 20) ? (cols - 20) : 8;

    nombre_objetivo = NULL;
    aparicion_objetivo = 0;
    if (ctx->simulacion_activa && ctx->simulacion_actual != NULL)
    {
        const SimTrackSnapshot *cursor_snapshot;

        nombre_objetivo = ctx->simulacion_actual->nombre;
        cursor_snapshot = ctx->simulacion_inicio;
        while (cursor_snapshot != NULL)
        {
            if (strcmp(cursor_snapshot->nombre, nombre_objetivo) == 0)
            {
                aparicion_objetivo++;
            }
            if (cursor_snapshot == ctx->simulacion_actual)
            {
                break;
            }
            cursor_snapshot = cursor_snapshot->sig;
        }
        if (aparicion_objetivo <= 0)
        {
            aparicion_objetivo = 1;
        }
    }

    while (actual != NULL && row < (height - 1))
    {
        int es_actual_reproduccion;

        es_actual_reproduccion = 0;
        if (nombre_objetivo != NULL)
        {
            if (strcmp(actual->nombre, nombre_objetivo) == 0)
            {
                aparicion_objetivo--;
                if (aparicion_objetivo == 0)
                {
                    es_actual_reproduccion = 1;
                    nombre_objetivo = NULL;
                }
            }
        }
        else if (!ctx->simulacion_activa && indice == 0)
        {
            es_actual_reproduccion = 1;
        }

        if (es_actual_reproduccion)
        {
            wattron(win_queue, COLOR_PAIR(3) | A_BOLD);
            mvwprintw(win_queue,
                      row,
                      2,
                      "> %-*.*s %6.2f",
                      name_w,
                      name_w,
                      actual->nombre,
                      actual->duracion);
            wattroff(win_queue, COLOR_PAIR(3) | A_BOLD);
        }
        else
        {
            mvwprintw(win_queue,
                      row,
                      2,
                      "  %-*.*s %6.2f",
                      name_w,
                      name_w,
                      actual->nombre,
                      actual->duracion);
        }

        actual = actual->sig;
        indice++;
        row++;
    }

    wrefresh(win_queue);
}

static void draw_status_panel(const UiContext *ctx)
{
    int color_modo;
    int height;
    int cols;
    int modo_x;
    int flags_x;
    int texto_w;
    const char *texto_estado;
    const char *texto_modo;
    char estado_simulacion[STATUS_MAX_LEN];

    getmaxyx(win_status, height, cols);
    (void)height;
    werase(win_status);
    box(win_status, 0, 0);

    if (ctx->simulacion_activa && ctx->simulacion_actual != NULL)
    {
        snprintf(estado_simulacion,
                 sizeof(estado_simulacion),
                 "reproduciendo %s [ciclo %d/%d]",
                 ctx->simulacion_actual->nombre,
                 ctx->simulacion_ciclo_actual,
                 (ctx->simulacion_ciclos == 0) ? 1 : ctx->simulacion_ciclos);
        texto_estado = estado_simulacion;
    }
    else
    {
        texto_estado = (ctx->reproduccion_estado[0] != '\0') ? ctx->reproduccion_estado : "reproduciendo --";
    }
    texto_modo = (ctx->reproduccion_modo[0] != '\0') ? ctx->reproduccion_modo : "IDLE";
    modo_x = (cols > 14) ? (cols - 12) : 2;
    flags_x = (cols > 34) ? (cols - 34) : 2;
    texto_w = (modo_x > 4) ? (modo_x - 4) : 1;

    wattron(win_status, COLOR_PAIR(3) | A_BOLD);
    mvwprintw(win_status, 1, 2, "%.*s", texto_w, texto_estado);
    wattroff(win_status, COLOR_PAIR(3) | A_BOLD);

    color_modo = color_para_modo(ctx->reproduccion_modo);
    wattron(win_status, (color_modo != 0 ? COLOR_PAIR(color_modo) : COLOR_PAIR(3)) | A_BOLD);
    mvwprintw(win_status, 1, modo_x, "[%s]", texto_modo);
    wattroff(win_status, (color_modo != 0 ? COLOR_PAIR(color_modo) : COLOR_PAIR(3)) | A_BOLD);

    if (ctx->reproduccion_fuente[0] != '\0' && cols > 42)
    {
        int fuente_x;

        fuente_x = (cols > 60) ? (cols / 2) : (modo_x - 18);
        if (fuente_x < 2)
        {
            fuente_x = 2;
        }
        mvwprintw(win_status, 1, fuente_x, "%s", ctx->reproduccion_fuente);
    }

    mvwprintw(win_status,
              1,
              flags_x,
              "mezcla:%s bucle:%s",
              ctx->coleccion.lista_reproduccion.shuffle_activado ? "si" : "no",
              ctx->coleccion.lista_reproduccion.loop_activado ? "si" : "no");

    wrefresh(win_status);
}

static void draw_log_panel(const UiContext *ctx)
{
    int height;
    int cols;
    int visibles;
    int indices[LOG_LINES_MAX];
    int cantidad;
    int pos;
    int idx_logico;
    int i;

    getmaxyx(win_log, height, cols);
    werase(win_log);
    box(win_log, 0, 0);
    wattron(win_log, COLOR_PAIR(2) | A_BOLD);
    mvwprintw(win_log,
              0,
              2,
              " eventos: %s ",
              ctx->log_solo_errores ? "solo errores" : "todos");
    wattroff(win_log, COLOR_PAIR(2) | A_BOLD);

    visibles = height - 2;
    if (visibles <= 0 || ctx->log_total <= 0)
    {
        wrefresh(win_log);
        return;
    }

    cantidad = 0;
    for (idx_logico = ctx->log_total - 1; idx_logico >= 0 && cantidad < visibles; idx_logico--)
    {
        int idx_fisico;

        idx_fisico = (ctx->log_inicio + idx_logico) % LOG_LINES_MAX;
        if (ctx->log_solo_errores && ctx->log_niveles[idx_fisico] != SALIDA_NIVEL_ERROR)
        {
            continue;
        }
        indices[cantidad] = idx_fisico;
        cantidad++;
    }

    for (i = 0; i < cantidad; i++)
    {
        int idx;

        pos = (cantidad - 1) - i;
        idx = indices[pos];
        if (ctx->log_niveles[idx] == SALIDA_NIVEL_ERROR)
        {
            wattron(win_log, COLOR_PAIR(2) | A_BOLD);
        }
        mvwprintw(win_log, 1 + i, 2, "%-*.*s", cols - 4, cols - 4, ctx->log_lineas[idx]);
        if (ctx->log_niveles[idx] == SALIDA_NIVEL_ERROR)
        {
            wattroff(win_log, COLOR_PAIR(2) | A_BOLD);
        }
    }

    wrefresh(win_log);
}

static void draw_command_panel(const UiContext *ctx)
{
    int rows;
    int cols;
    int cmd_x;
    int y;

    getmaxyx(win_cmd, rows, cols);
    (void)rows;
    werase(win_cmd);
    box(win_cmd, 0, 0);

    y = 1;
    draw_hotkey(win_cmd, y, 2, 'b', "atras");
    draw_hotkey(win_cmd, y, 17, 'n', "siguiente");
    draw_hotkey(win_cmd, y, 36, 's', "mezclar");
    draw_hotkey(win_cmd, y, 53, 'l', "bucle");
    draw_hotkey(win_cmd, y, 67, 'c', "limpiar");
    draw_hotkey(win_cmd, y, 84, 'e', "eventos");
    draw_hotkey(win_cmd, y, 101, 'q', "salir");

    mvwprintw(win_cmd, 0, 2, " %s ", ctx->estado);

    cmd_x = (cols > 72) ? (cols / 2) : 40;
    mvwprintw(win_cmd, 1, cmd_x, "comando> %-48.48s", ctx->comando);
    if (cols > 26)
    {
        mvwprintw(win_cmd, 0, cols - 20, "Enter: ejecutar");
    }

    wrefresh(win_cmd);
}

int ui_is_predefined_command(const char *comando)
{
    char buffer[CMD_MAX_LEN];
    char *parametro;

    if (comando == NULL)
    {
        return 0;
    }

    strncpy(buffer, comando, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    trim_line_end(buffer);
    parametro = skip_leading_space(buffer);
    if (*parametro == '\0')
    {
        return 0;
    }

    if (strcmp(parametro, "q") == 0 || strcmp(parametro, "quit") == 0 ||
        strcmp(parametro, "next") == 0 || strcmp(parametro, "back") == 0 ||
        strcmp(parametro, "shuffle") == 0 || strcmp(parametro, "loop") == 0 ||
        strcmp(parametro, "clear_queue") == 0 || strcmp(parametro, "catalog") == 0 ||
        strcmp(parametro, "lists") == 0 || strcmp(parametro, "list") == 0 ||
        strcmp(parametro, "load") == 0 || strncmp(parametro, "load ", 5) == 0)
    {
        return 1;
    }

    if (strncmp(parametro, "play", 4) == 0 ||
        strncmp(parametro, "queue", 5) == 0 ||
        strncmp(parametro, "songs", 5) == 0 ||
        strncmp(parametro, "new", 3) == 0)
    {
        return 1;
    }

    return 0;
}

static int total_listas_ui(const UiContext *ctx)
{
    int total;

    total = 1 + ctx->coleccion.total_listas;
    if (total <= 0)
    {
        return 1;
    }

    return total;
}

static const ListaDoble *lista_por_indice(const UiContext *ctx, int indice, const char **nombre)
{
    NodoListaReproduccion *actual;
    int i;

    if (indice <= 0)
    {
        if (nombre != NULL)
        {
            *nombre = "Catalogo";
        }
        return &ctx->coleccion.catalogo;
    }

    actual = ctx->coleccion.listas;
    i = 1;
    while (actual != NULL)
    {
        if (i == indice)
        {
            if (nombre != NULL)
            {
                *nombre = actual->nombre;
            }
            return &actual->canciones;
        }
        actual = actual->sig;
        i++;
    }

    if (nombre != NULL)
    {
        *nombre = "Catalogo";
    }
    return &ctx->coleccion.catalogo;
}

static const Cancion *cancion_por_indice(const ListaDoble *lista, int indice)
{
    const Cancion *actual;
    int i;

    if (lista == NULL || indice < 0)
    {
        return NULL;
    }

    actual = lista->cabeza;
    i = 0;
    while (actual != NULL)
    {
        if (i == indice)
        {
            return actual;
        }
        actual = actual->sig;
        i++;
    }

    return NULL;
}

static void set_estado(UiContext *ctx, const char *mensaje)
{
    set_estado_nivel(ctx, SALIDA_NIVEL_INFO, mensaje);
}

static void set_estado_nivel(UiContext *ctx, SalidaNivel nivel, const char *mensaje)
{
    size_t largo;

    if (mensaje == NULL)
    {
        return;
    }

    strncpy(ctx->estado, mensaje, sizeof(ctx->estado) - 1);
    ctx->estado[sizeof(ctx->estado) - 1] = '\0';

    largo = strlen(ctx->estado);
    while (largo > 0 && (ctx->estado[largo - 1] == '\n' || ctx->estado[largo - 1] == '\r'))
    {
        ctx->estado[largo - 1] = '\0';
        largo--;
    }

    if (ctx->estado[0] != '\0')
    {
        ui_append_log(ctx, nivel, ctx->estado);
    }
}

static void set_reproduccion_estado(UiContext *ctx, const char *mensaje)
{
    if (ctx == NULL || mensaje == NULL)
    {
        return;
    }

    strncpy(ctx->reproduccion_estado, mensaje, sizeof(ctx->reproduccion_estado) - 1);
    ctx->reproduccion_estado[sizeof(ctx->reproduccion_estado) - 1] = '\0';
}

static void set_reproduccion_modo(UiContext *ctx, const char *modo)
{
    if (ctx == NULL || modo == NULL)
    {
        return;
    }

    strncpy(ctx->reproduccion_modo, modo, sizeof(ctx->reproduccion_modo) - 1);
    ctx->reproduccion_modo[sizeof(ctx->reproduccion_modo) - 1] = '\0';
}

static void set_reproduccion_fuente(UiContext *ctx, const char *fuente)
{
    if (ctx == NULL || fuente == NULL)
    {
        return;
    }

    strncpy(ctx->reproduccion_fuente, fuente, sizeof(ctx->reproduccion_fuente) - 1);
    ctx->reproduccion_fuente[sizeof(ctx->reproduccion_fuente) - 1] = '\0';
}

static int color_para_modo(const char *modo)
{
    if (modo == NULL)
    {
        return 0;
    }

    if (strcmp(modo, "PLAY") == 0)
    {
        return 3;
    }

    if (strcmp(modo, "QUEUE") == 0)
    {
        return 4;
    }

    if (strcmp(modo, "SHUFFLE") == 0)
    {
        return 1;
    }

    if (strcmp(modo, "LOOP") == 0)
    {
        return 2;
    }

    return 0;
}

static void liberar_simulacion(UiContext *ctx)
{
    SimTrackSnapshot *actual;
    SimTrackSnapshot *siguiente;

    if (ctx == NULL)
    {
        return;
    }

    actual = ctx->simulacion_inicio;
    while (actual != NULL)
    {
        siguiente = actual->sig;
        free(actual);
        actual = siguiente;
    }

    ctx->simulacion_inicio = NULL;
    ctx->simulacion_actual = NULL;
    ctx->simulacion_fin = NULL;
    ctx->simulacion_punto_loop = NULL;
    ctx->simulacion_activa = 0;
    ctx->simulacion_total = 0;
    ctx->simulacion_indice = 0;
    ctx->simulacion_ciclo_actual = 0;
    ctx->simulacion_ciclos = 0;
    ctx->simulacion_ultimo_tick = 0;
}

static int agregar_snapshost_simulacion(UiContext *ctx, const char *nombre, float duracion)
{
    SimTrackSnapshot *nuevo;

    if (ctx == NULL || nombre == NULL)
    {
        return -1;
    }

    nuevo = (SimTrackSnapshot *)malloc(sizeof(SimTrackSnapshot));
    if (nuevo == NULL)
    {
        return -1;
    }

    strncpy(nuevo->nombre, nombre, sizeof(nuevo->nombre) - 1);
    nuevo->nombre[sizeof(nuevo->nombre) - 1] = '\0';
    nuevo->duracion = duracion;
    nuevo->sig = NULL;

    if (ctx->simulacion_inicio == NULL)
    {
        ctx->simulacion_inicio = nuevo;
        ctx->simulacion_fin = nuevo;
    }
    else
    {
        ctx->simulacion_fin->sig = nuevo;
        ctx->simulacion_fin = nuevo;
    }

    return 0;
}

static int contar_snapshot_simulacion(const UiContext *ctx)
{
    const SimTrackSnapshot *actual;
    int total;

    if (ctx == NULL)
    {
        return 0;
    }

    total = 0;
    actual = ctx->simulacion_inicio;
    while (actual != NULL)
    {
        total++;
        actual = actual->sig;
    }

    return total;
}

static void ui_append_log(UiContext *ctx, SalidaNivel nivel, const char *mensaje)
{
    int idx;

    if (ctx == NULL || mensaje == NULL || mensaje[0] == '\0')
    {
        return;
    }

    if (ctx->log_total < LOG_LINES_MAX)
    {
        idx = (ctx->log_inicio + ctx->log_total) % LOG_LINES_MAX;
        ctx->log_total++;
    }
    else
    {
        idx = ctx->log_inicio;
        ctx->log_inicio = (ctx->log_inicio + 1) % LOG_LINES_MAX;
    }

    strncpy(ctx->log_lineas[idx], mensaje, STATUS_MAX_LEN - 1);
    ctx->log_lineas[idx][STATUS_MAX_LEN - 1] = '\0';
    ctx->log_niveles[idx] = nivel;
}

static void ui_salida_handler(SalidaNivel nivel, const char *mensaje, void *userdata)
{
    UiContext *ctx;

    if (mensaje == NULL || userdata == NULL)
    {
        return;
    }

    ctx = (UiContext *)userdata;
    set_estado_nivel(ctx, nivel, mensaje);
}

// Esta función prepara la simulación de reproducción de manera NO BLOQUEANTE.
// Nunca debe bloquear la UI ni impedir la ejecución de otros comandos.
// Se apoya en avanzar_simulacion_ux(), que es llamada en cada ciclo del loop principal.
static void simular_reproduccion_ux(UiContext *ctx, int omitir_actual)
{
    const Cancion *actual;

    if (ctx == NULL)
    {
        return;
    }


    // Garantiza que la simulación nunca bloquee la UI ni la entrada de comandos.
    liberar_simulacion(ctx);

    actual = ctx->coleccion.lista_reproduccion.canciones.cabeza;
    if (omitir_actual && actual != NULL)
    {
        actual = actual->sig;
    }

    // Solo prepara la lista de reproducción a simular, sin bucles ni esperas.
    while (actual != NULL)
    {
        if (agregar_snapshost_simulacion(ctx, actual->nombre, actual->duracion) != 0)
        {
            liberar_simulacion(ctx);
            set_estado_nivel(ctx, SALIDA_NIVEL_ERROR, "No se pudo preparar la simulacion de reproduccion.");
            set_reproduccion_estado(ctx, "reproduciendo --");
            set_reproduccion_modo(ctx, "IDLE");
            set_reproduccion_fuente(ctx, "");
            return;
        }
        actual = actual->sig;
    }

    ctx->simulacion_total = contar_snapshot_simulacion(ctx);
    if (ctx->simulacion_total <= 0)
    {
        set_estado_nivel(ctx, SALIDA_NIVEL_ERROR, "No hay canciones para simular reproduccion.");
        set_reproduccion_estado(ctx, "reproduciendo --");
        set_reproduccion_modo(ctx, "IDLE");
        set_reproduccion_fuente(ctx, "");
        return;
    }

    ctx->simulacion_activa = 1;
    ctx->simulacion_omitir_actual = omitir_actual;
    ctx->simulacion_indice = 1;
    ctx->simulacion_ciclo_actual = 1;
    ctx->simulacion_ciclos = ctx->coleccion.lista_reproduccion.loop_activado ? 0 : 1;
    ctx->simulacion_actual = ctx->simulacion_inicio;
    ctx->simulacion_punto_loop = ctx->simulacion_actual;
    ctx->simulacion_ultimo_tick = time(NULL);

    if (ctx->coleccion.lista_reproduccion.shuffle_activado)
    {
        set_estado(ctx, "Shuffle activo: reproduccion en orden mezclado actual.");
        set_reproduccion_estado(ctx, "reproduciendo | shuffle activo");
        set_reproduccion_modo(ctx, "SHUFFLE");
        redraw_all(ctx);
    }

    if (ctx->coleccion.lista_reproduccion.loop_activado)
    {
        set_estado(ctx, "Loop activo: la simulacion repetira una vez la cola.");
        set_reproduccion_estado(ctx, "reproduciendo | loop activo");
        set_reproduccion_modo(ctx, "LOOP");
        redraw_all(ctx);
    }

    if (ctx->simulacion_actual != NULL)
    {
        snprintf(ctx->reproduccion_estado,
                 sizeof(ctx->reproduccion_estado),
                 "reproduciendo %s [ciclo %d/%d]",
                 ctx->simulacion_actual->nombre,
                 ctx->simulacion_ciclo_actual,
                 ctx->simulacion_ciclos);
    }

    redraw_all(ctx);
}

static void avanzar_simulacion_ux(UiContext *ctx)
{
    SimTrackSnapshot *siguiente;
    time_t ahora;


    // Esta función avanza la simulación de manera NO BLOQUEANTE.
    // Se llama en cada ciclo del loop principal, nunca bloquea la UI.
    if (ctx == NULL || !ctx->simulacion_activa || ctx->simulacion_actual == NULL)
    {
        return;
    }

    ahora = time(NULL);
    if (difftime(ahora, ctx->simulacion_ultimo_tick) < 3.0)
    {
        return;
    }

    ctx->simulacion_ultimo_tick = ahora;
    if (ctx->simulacion_indice >= ctx->simulacion_total)
    {
        if (ctx->coleccion.lista_reproduccion.loop_activado ||
            ctx->simulacion_ciclos == 0 ||
            ctx->simulacion_ciclo_actual < ctx->simulacion_ciclos)
        {
            ctx->simulacion_ciclo_actual++;
            ctx->simulacion_indice = 1;
            if (ctx->coleccion.lista_reproduccion.loop_activado && ctx->simulacion_punto_loop != NULL)
            {
                ctx->simulacion_actual = ctx->simulacion_punto_loop;
            }
            else
            {
                ctx->simulacion_actual = ctx->simulacion_inicio;
            }
        }
        else
        {
            ctx->simulacion_activa = 0;
            set_estado(ctx, "Simulacion de reproduccion finalizada.");
            set_reproduccion_estado(ctx, "reproduciendo --");
            set_reproduccion_modo(ctx, "IDLE");
            set_reproduccion_fuente(ctx, "");
            liberar_simulacion(ctx);
            redraw_all(ctx);
            return;
        }
    }
    else
    {
        siguiente = ctx->simulacion_actual->sig;
        if (siguiente == NULL)
        {
            siguiente = ctx->simulacion_inicio;
        }

        ctx->simulacion_actual = siguiente;
        ctx->simulacion_indice++;
    }

    if (ctx->simulacion_actual != NULL)
    {
        snprintf(ctx->reproduccion_estado,
                 sizeof(ctx->reproduccion_estado),
                 "reproduciendo %s [ciclo %d/%d]",
                 ctx->simulacion_actual->nombre,
                 ctx->simulacion_ciclo_actual,
                 ctx->simulacion_ciclos);
        redraw_all(ctx);
    }
}

static void ejecutar_comando(UiContext *ctx, const char *comando)
{
    char buffer[CMD_MAX_LEN];
    char *parametro;

    if (!ui_is_predefined_command(comando))
    {
        set_estado_nivel(ctx, SALIDA_NIVEL_ERROR, "Comando no permitido.");
        return;
    }

    strncpy(buffer, comando, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    trim_line_end(buffer);
    parametro = skip_leading_space(buffer);
    if (*parametro == '\0')
    {
        set_estado_nivel(ctx, SALIDA_NIVEL_ERROR, "Comando vacio.");
        return;
    }

    if (strcmp(parametro, "q") == 0 || strcmp(parametro, "quit") == 0)
    {
        ctx->salir = 1;
        return;
    }

    if (strncmp(parametro, "play", 4) == 0)
    {
        if (reproducir_nueva_cancion_o_lista(&ctx->coleccion, parametro) == 0)
        {
            set_estado(ctx, "Iniciando Reproducción");
            set_reproduccion_modo(ctx, "PLAY");
            set_reproduccion_fuente(ctx, "PLAY");
            {
                const Cancion *actual;

                actual = ctx->coleccion.lista_reproduccion.canciones.cabeza;
                if (actual != NULL)
                {
                    snprintf(ctx->reproduccion_estado,
                             sizeof(ctx->reproduccion_estado),
                             "reproduciendo %s [ciclo 1/1]",
                             actual->nombre);
                }
                else
                {
                    set_reproduccion_estado(ctx, "reproduciendo --");
                }
            }
            simular_reproduccion_ux(ctx, 0);
        }
        else
        {
            set_estado_nivel(ctx, SALIDA_NIVEL_ERROR, "Fallo al ejecutar play.");
        }
        return;
    }

    if (strncmp(parametro, "queue", 5) == 0)
    {
        int omitir_actual;

        omitir_actual = (ctx->coleccion.lista_reproduccion.canciones.cabeza != NULL) ? 1 : 0;
        if (agregar_a_cola_reproduccion(&ctx->coleccion, parametro) == 0)
        {
            set_estado(ctx, "Continuando Simulación");
            set_reproduccion_modo(ctx, "QUEUE");
            set_reproduccion_fuente(ctx, "QUEUE");
            {
                const Cancion *actual;

                actual = ctx->coleccion.lista_reproduccion.canciones.cabeza;
                if (actual != NULL)
                {
                    snprintf(ctx->reproduccion_estado,
                             sizeof(ctx->reproduccion_estado),
                             "reproduciendo %s [ciclo 1/1]",
                             actual->nombre);
                }
                else
                {
                    set_reproduccion_estado(ctx, "reproduciendo --");
                }
            }
            if (!ctx->simulacion_activa)
            {
                simular_reproduccion_ux(ctx, omitir_actual);
            }
        }
        else
        {
            set_estado_nivel(ctx, SALIDA_NIVEL_ERROR, "Fallo al ejecutar queue.");
        }
        return;
    }

    if (strcmp(parametro, "next") == 0)
    {
        next(&ctx->coleccion);
        set_estado(ctx, "Comando next ejecutado.");
        set_reproduccion_modo(ctx, "PLAY");
        set_reproduccion_fuente(ctx, "PLAY");
        if (ctx->coleccion.lista_reproduccion.canciones.cabeza != NULL)
        {
            snprintf(ctx->reproduccion_estado,
                     sizeof(ctx->reproduccion_estado),
                     "reproduciendo %s",
                     ctx->coleccion.lista_reproduccion.canciones.cabeza->nombre);
        }
        else
        {
            set_reproduccion_estado(ctx, "reproduciendo --");
        }
        return;
    }

    if (strcmp(parametro, "back") == 0)
    {
        back(&ctx->coleccion);
        set_estado(ctx, "Comando back ejecutado.");
        set_reproduccion_modo(ctx, "PLAY");
        set_reproduccion_fuente(ctx, "PLAY");
        if (ctx->coleccion.lista_reproduccion.canciones.cabeza != NULL)
        {
            snprintf(ctx->reproduccion_estado,
                     sizeof(ctx->reproduccion_estado),
                     "reproduciendo %s",
                     ctx->coleccion.lista_reproduccion.canciones.cabeza->nombre);
        }
        else
        {
            set_reproduccion_estado(ctx, "reproduciendo --");
        }
        // Reinicia la simulación UX desde la canción actual
        simular_reproduccion_ux(ctx, 0);
        return;
    }

    if (strcmp(parametro, "shuffle") == 0)
    {
        shuffle(&ctx->coleccion);
        set_estado(ctx, "Comando shuffle ejecutado.");
        set_reproduccion_modo(ctx, "SHUFFLE");
        set_reproduccion_fuente(ctx, "SHUFFLE");
        simular_reproduccion_ux(ctx, 0);
        return;
    }

    if (strcmp(parametro, "loop") == 0)
    {
        loop(&ctx->coleccion);
        set_estado(ctx, "Comando loop ejecutado.");
        set_reproduccion_modo(ctx, "LOOP");
        set_reproduccion_fuente(ctx, "LOOP");

        // Reinicia la simulación desde la canción actual
        simular_reproduccion_ux(ctx, 0);

        // Actualiza los paneles de reproducción y cola
        redraw_all(ctx);
        return;
    }

    if (strcmp(parametro, "clear_queue") == 0)
    {
        clear_queue(&ctx->coleccion);
        set_estado(ctx, "Comando clear_queue ejecutado.");
        set_reproduccion_modo(ctx, "IDLE");
        set_reproduccion_fuente(ctx, "IDLE");
        if (ctx->coleccion.lista_reproduccion.canciones.cabeza != NULL)
        {
            snprintf(ctx->reproduccion_estado,
                     sizeof(ctx->reproduccion_estado),
                     "reproduciendo %s",
                     ctx->coleccion.lista_reproduccion.canciones.cabeza->nombre);
        }
        else
        {
            set_reproduccion_estado(ctx, "reproduciendo --");
        }
        return;
    }

    if (strcmp(parametro, "catalog") == 0)
    {
        ctx->indice_lista = 0;
        ctx->indice_track = 0;
        set_estado(ctx, "Vista en catalogo.");
        set_reproduccion_modo(ctx, "PLAY");
        set_reproduccion_fuente(ctx, "PLAY");
        return;
    }

    if (strcmp(parametro, "lists") == 0 || strcmp(parametro, "list") == 0)
    {
        ctx->foco_panel = 0;
        set_estado(ctx, "Vista en listas.");
        set_reproduccion_modo(ctx, "PLAY");
        set_reproduccion_fuente(ctx, "PLAY");
        return;
    }

    if (strncmp(parametro, "songs", 5) == 0)
    {
        char *nombre_lista;
        NodoListaReproduccion *actual;
        int idx;

        nombre_lista = parametro + 5;
        nombre_lista = skip_leading_space(nombre_lista);
        if (*nombre_lista == '\0')
        {
            set_estado_nivel(ctx, SALIDA_NIVEL_ERROR, "Uso: songs <nombre_lista>");
            return;
        }

        idx = 1;
        actual = ctx->coleccion.listas;
        while (actual != NULL)
        {
            if (strcmp(actual->nombre, nombre_lista) == 0)
            {
                ctx->indice_lista = idx;
                ctx->indice_track = 0;
                ctx->foco_panel = 1;
                set_estado(ctx, "Lista seleccionada desde songs.");
                return;
            }
            actual = actual->sig;
            idx++;
        }

        set_estado_nivel(ctx, SALIDA_NIVEL_ERROR, "Lista no encontrada.");
        return;
    }

    if (strncmp(parametro, "new", 3) == 0)
    {
        (void)cargar_nueva_lista_en_ejecucion(&ctx->coleccion, ctx->ruta_actual, parametro);
        set_estado(ctx, "Comando new ejecutado.");
        return;
    }

    if (strncmp(parametro, "load", 4) == 0 &&
        (parametro[4] == '\0' || isspace((unsigned char)parametro[4])))
    {
        int usar_prefijo_data;
        int rc_load;
        char mensaje_load[STATUS_MAX_LEN];
        char ruta_data[PATH_MAX_LEN];
        const char *ruta_objetivo;
        char *ruta_load;

        ruta_load = parametro + 4;
        ruta_load = skip_leading_space(ruta_load);

        usar_prefijo_data = 0;
        ruta_objetivo = ruta_load;
        if (*ruta_load != '\0' &&
            strchr(ruta_load, '/') == NULL &&
            strchr(ruta_load, '\\') == NULL &&
            strchr(ruta_load, ':') == NULL)
        {
            snprintf(ruta_data, sizeof(ruta_data), "data/%s", ruta_load);
            ruta_objetivo = ruta_data;
            usar_prefijo_data = 1;
        }

        liberar_coleccion_musical(&ctx->coleccion);
        inicializar_coleccion_musical(&ctx->coleccion);
        rc_load = cargar_coleccion_desde_ruta(
                &ctx->coleccion,
                (*ruta_load == '\0') ? "" : ruta_objetivo,
                ctx->ruta_actual,
                sizeof(ctx->ruta_actual));

        if (rc_load != 0 && usar_prefijo_data)
        {
            rc_load = cargar_coleccion_desde_ruta(
                &ctx->coleccion,
                ruta_load,
                ctx->ruta_actual,
                sizeof(ctx->ruta_actual));
        }

        if (rc_load == 0)
        {
            ctx->indice_lista = 0;
            ctx->indice_track = 0;
            snprintf(mensaje_load, sizeof(mensaje_load), "Coleccion recargada desde: %.160s", ctx->ruta_actual);
            set_estado(ctx, mensaje_load);
        }
        else
        {
            set_estado_nivel(ctx, SALIDA_NIVEL_ERROR, "Error al recargar coleccion.");
        }
        return;
    }

    set_estado_nivel(ctx, SALIDA_NIVEL_ERROR, "Comando no permitido.");
}

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

static void draw_hotkey(WINDOW *win, int y, int x, char key, const char *label)
{
    wattron(win, COLOR_PAIR(1) | A_BOLD);
    mvwprintw(win, y, x, "[Alt+%c]", key);
    wattroff(win, COLOR_PAIR(1) | A_BOLD);
    wattron(win, COLOR_PAIR(5));
    mvwprintw(win, y, x + 8, "%s", label);
    wattroff(win, COLOR_PAIR(5));
}

static void cleanup(void)
{
    if (win_artists != NULL)
    {
        delwin(win_artists);
    }
    if (win_tracks != NULL)
    {
        delwin(win_tracks);
    }
    if (win_queue != NULL)
    {
        delwin(win_queue);
    }
    if (win_status != NULL)
    {
        delwin(win_status);
    }
    if (win_log != NULL)
    {
        delwin(win_log);
    }
    if (win_cmd != NULL)
    {
        delwin(win_cmd);
    }
    endwin();
}
