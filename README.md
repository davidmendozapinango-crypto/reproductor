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

## Checklist QA Manual

Precondicion:

1. Compilar el proyecto:

```bat
cmd /c build.bat
```

2. Ejecutar el programa:

```powershell
.\build\reproductor.exe
```

### Matriz de pruebas

- [ ] Carga por defecto: presionar Enter en la ruta inicial y ejecutar `lists`.
- [ ] Carga por ruta explicita: ingresar una ruta completa valida y ejecutar `catalog`.
- [ ] Recarga por defecto: ejecutar `load`, presionar Enter y luego `catalog`.
- [ ] Recarga por ruta explicita: ejecutar `load`, ingresar ruta valida y luego `lists`.
- [ ] Fallback de recarga: ejecutar `load` con ruta invalida y verificar que carga por defecto.
- [ ] Listado de canciones: ejecutar `songs Playlist1` y validar salida.
- [ ] Lista inexistente: ejecutar `songs ListaNoExiste` y validar mensaje de error.
- [ ] Reproducir cancion: ejecutar `play Back in Black`, luego `current`.
- [ ] Reproducir lista: ejecutar `play Playlist1`, luego `current`.
- [ ] Crear lista valida: ejecutar `new "PlaylistQA": Back in Black - Born For This`, luego `lists`.
- [ ] Crear lista invalida: ejecutar `new "PlaylistBad": CancionInventada - Back in Black`.
- [ ] Persistencia de lista: crear lista con `new`, salir con `q` y revisar que se anexo al final del archivo activo.
- [ ] Salida limpia: ejecutar `q` y validar cierre sin errores.

## Regresion automatizada

Ejecutar:

```bat
cmd /c tests\run_tests.bat
```

Resultado esperado:

- [ ] Finaliza con `Tests passed: build\test_utils.exe`.
