/*
 * ui.c
 * Interfaz de usuario de terminal con NCURSES.
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
#include <string.h>

#include "cancion.h"
#include "ui.h"

#define CMD_MAX_LEN 256
#define STATUS_MAX_LEN 192
#define PATH_MAX_LEN 512

typedef struct UiContext
{
    ColeccionMusical coleccion;
    char ruta_actual[PATH_MAX_LEN];
    char comando[CMD_MAX_LEN];
    char estado[STATUS_MAX_LEN];
    size_t comando_len;
    int indice_lista;
    int indice_track;
    int foco_panel;
    int salir;
} UiContext;

static WINDOW *win_artists;
static WINDOW *win_tracks;
static WINDOW *win_queue;
static WINDOW *win_status;
static WINDOW *win_cmd;

static void draw_layout(void);
static void cleanup(void);
static void handle_input(UiContext *ctx);
static void redraw_all(const UiContext *ctx);
static void draw_artists_panel(const UiContext *ctx);
static void draw_tracks_panel(const UiContext *ctx);
static void draw_queue_panel(const UiContext *ctx);
static void draw_status_panel(const UiContext *ctx);
static void draw_command_panel(const UiContext *ctx);
static int total_listas_ui(const UiContext *ctx);
static const ListaDoble *lista_por_indice(const UiContext *ctx, int indice, const char **nombre);
static const Cancion *cancion_por_indice(const ListaDoble *lista, int indice);
static void set_estado(UiContext *ctx, const char *mensaje);
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
        set_estado(&ctx, "No se pudo cargar el catalogo inicial.");
    }
    else
    {
        set_estado(&ctx, "Coleccion cargada. Usa Ctrl+P para ejecutar comandos.");
    }

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
    int cmd_h;
    int main_h;

    getmaxyx(stdscr, rows, cols);
    artists_w = cols / 4;
    tracks_w = cols - artists_w;
    status_h = 3;
    cmd_h = 3;
    main_h = rows - status_h - cmd_h;

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
    win_cmd = newwin(cmd_h, cols, main_h + status_h, 0);

    keypad(win_cmd, TRUE);
    wbkgd(win_artists, COLOR_PAIR(5));
    wbkgd(win_tracks, COLOR_PAIR(5));
    wbkgd(win_queue, COLOR_PAIR(5));
    wbkgd(win_status, COLOR_PAIR(4));
    wbkgd(win_cmd, COLOR_PAIR(5));
}

static void handle_input(UiContext *ctx)
{
    int ch;

    while (!ctx->salir)
    {
        redraw_all(ctx);
        ch = wgetch(win_cmd);

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

        if (ch == 16)
        {
            if (ctx->comando_len == 0)
            {
                set_estado(ctx, "No hay comando para ejecutar.");
            }
            else
            {
                ejecutar_comando(ctx, ctx->comando);
                ctx->comando[0] = '\0';
                ctx->comando_len = 0;
            }
            continue;
        }

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
                    set_estado(ctx, "Pista seleccionada. Ejecuta con Ctrl+P.");
                }
                else
                {
                    set_estado(ctx, "No hay pista seleccionada para reproducir.");
                }
            }
            else
            {
                set_estado(ctx, "Usa Ctrl+P para ejecutar el comando escrito.");
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
    while (actual != NULL && row < (height - 1))
    {
        if (indice == 0)
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
    const Cancion *actual;

    werase(win_status);
    box(win_status, 0, 0);

    actual = ctx->coleccion.lista_reproduccion.canciones.cabeza;
    if (actual != NULL)
    {
        wattron(win_status, COLOR_PAIR(3) | A_BOLD);
        mvwprintw(win_status, 1, 2, "reproduciendo %s (%.2f)", actual->nombre, actual->duracion);
        wattroff(win_status, COLOR_PAIR(3) | A_BOLD);
    }
    else
    {
        mvwprintw(win_status, 1, 2, "reproduciendo --");
    }

    mvwprintw(win_status,
              1,
              46,
              "mezcla:%s bucle:%s",
              ctx->coleccion.lista_reproduccion.shuffle_activado ? "si" : "no",
              ctx->coleccion.lista_reproduccion.loop_activado ? "si" : "no");

    wrefresh(win_status);
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
    draw_hotkey(win_cmd, y, 84, 'q', "salir");

    mvwprintw(win_cmd, 0, 2, " %s ", ctx->estado);

    cmd_x = (cols > 72) ? (cols / 2) : 40;
    mvwprintw(win_cmd, 1, cmd_x, "comando> %-48.48s", ctx->comando);
    if (cols > 26)
    {
        mvwprintw(win_cmd, 0, cols - 24, "Ctrl+P: ejecutar");
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
        strcmp(parametro, "load") == 0)
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
    strncpy(ctx->estado, mensaje, sizeof(ctx->estado) - 1);
    ctx->estado[sizeof(ctx->estado) - 1] = '\0';
}

static void ejecutar_comando(UiContext *ctx, const char *comando)
{
    char buffer[CMD_MAX_LEN];
    char *parametro;

    if (!ui_is_predefined_command(comando))
    {
        set_estado(ctx, "Comando no permitido.");
        return;
    }

    strncpy(buffer, comando, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    trim_line_end(buffer);
    parametro = skip_leading_space(buffer);
    if (*parametro == '\0')
    {
        set_estado(ctx, "Comando vacio.");
        return;
    }

    if (strcmp(parametro, "q") == 0 || strcmp(parametro, "quit") == 0)
    {
        ctx->salir = 1;
        return;
    }

    if (strncmp(parametro, "play", 4) == 0)
    {
        (void)reproducir_nueva_cancion_o_lista(&ctx->coleccion, parametro);
        set_estado(ctx, "Comando play ejecutado.");
        return;
    }

    if (strncmp(parametro, "queue", 5) == 0)
    {
        (void)agregar_a_cola_reproduccion(&ctx->coleccion, parametro);
        set_estado(ctx, "Comando queue ejecutado.");
        return;
    }

    if (strcmp(parametro, "next") == 0)
    {
        next(&ctx->coleccion);
        set_estado(ctx, "Comando next ejecutado.");
        return;
    }

    if (strcmp(parametro, "back") == 0)
    {
        back(&ctx->coleccion);
        set_estado(ctx, "Comando back ejecutado.");
        return;
    }

    if (strcmp(parametro, "shuffle") == 0)
    {
        shuffle(&ctx->coleccion);
        set_estado(ctx, "Comando shuffle ejecutado.");
        return;
    }

    if (strcmp(parametro, "loop") == 0)
    {
        loop(&ctx->coleccion);
        set_estado(ctx, "Comando loop ejecutado.");
        return;
    }

    if (strcmp(parametro, "clear_queue") == 0)
    {
        clear_queue(&ctx->coleccion);
        set_estado(ctx, "Comando clear_queue ejecutado.");
        return;
    }

    if (strcmp(parametro, "catalog") == 0)
    {
        ctx->indice_lista = 0;
        ctx->indice_track = 0;
        set_estado(ctx, "Vista en catalogo.");
        return;
    }

    if (strcmp(parametro, "lists") == 0 || strcmp(parametro, "list") == 0)
    {
        ctx->foco_panel = 0;
        set_estado(ctx, "Vista en listas.");
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
            set_estado(ctx, "Uso: songs <nombre_lista>");
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

        set_estado(ctx, "Lista no encontrada.");
        return;
    }

    if (strncmp(parametro, "new", 3) == 0)
    {
        (void)cargar_nueva_lista_en_ejecucion(&ctx->coleccion, ctx->ruta_actual, parametro);
        set_estado(ctx, "Comando new ejecutado.");
        return;
    }

    if (strcmp(parametro, "load") == 0)
    {
        liberar_coleccion_musical(&ctx->coleccion);
        inicializar_coleccion_musical(&ctx->coleccion);
        if (cargar_coleccion_desde_ruta(&ctx->coleccion, "", ctx->ruta_actual, sizeof(ctx->ruta_actual)) == 0)
        {
            ctx->indice_lista = 0;
            ctx->indice_track = 0;
            set_estado(ctx, "Coleccion recargada.");
        }
        else
        {
            set_estado(ctx, "Error al recargar coleccion.");
        }
        return;
    }

    set_estado(ctx, "Comando no permitido.");
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
    if (win_cmd != NULL)
    {
        delwin(win_cmd);
    }
    endwin();
}
