#ifndef UTILS_H
#define UTILS_H

void imprimir_bienvenida(void);
#endif /* UTILS_H */

void imprimir_catalogo(){//Abre e imrpimir el Ctalogo con las canciones y playlits
    FILE *arch;
    char line[100];
    arch = fopen("Catalogo.txt", "r");
    if (arch == NULL)
    {
        printf("Error al abrir el archivo de Catalogo.\n");
        return;
    }
    while ((fgets(line, 100, arch)) != NULL)
    {
        printf("%s", line);
    }
    fclose(arch);
}