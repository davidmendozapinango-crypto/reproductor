# reproductor

## Build en Windows

Usa `build.bat` desde `cmd` o desde la tarea de VS Code llamada `build reproductor`.

Requisito: tener un compilador C completo en el PATH. En esta máquina `make.exe` no es necesario para compilar el proyecto.

Si tienes Windows 10/11:
🔹 Instalar WSL (Ubuntu)
> Abre PowerShell como administrador:
´´´PowerShell
    wsl --installMostrar más líneas
´´´
Reinicia y abre Ubuntu.
Instala herramientas:

´´´sh
    sudo apt update
    sudo apt install build-essential
´´´

▶️ Usar Makefile en WSL
´´´
make./proyectoMostrar más líneas
´´´
