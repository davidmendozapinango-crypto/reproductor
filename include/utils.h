
#ifndef UTILS_H
#define UTILS_H

void imprimir_bienvenida(void);
void Menu();
#endif /* UTILS_H */

void Menu(){//Imprimir menu
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
//#MOVER
