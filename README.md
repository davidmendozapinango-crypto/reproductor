# reproductor

Proyecto en C para simular un motor de reproduccion musical con catalogo global,
listas de reproduccion y comandos interactivos por consola.

## Build en Windows

Compilar desde la raiz del proyecto:

```bat
cmd /c build.bat
```

Ejecutar:

```powershell
.\build\reproductor.exe
```

Nota:
- Se requiere GCC en el PATH.
- `make.exe` no es obligatorio para el flujo actual de compilacion en Windows.

## Build en WSL (opcional)

Si prefieres compilar con WSL:

```powershell
wsl --install
```

Luego, en Ubuntu:

```bash
sudo apt update
sudo apt install build-essential
make
```

## Checklist QA manual

Precondicion:
1. Ejecutar `cmd /c build.bat`.
2. Ejecutar `.\build\reproductor.exe`.

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

## Troubleshooting

- Error: `Permission denied` al compilar `build\reproductor.exe`.
    Causa probable: el ejecutable sigue abierto.
    Solucion: cerrar la instancia en ejecucion y volver a correr `cmd /c build.bat`.

- Error: `no se pudo abrir el archivo en la ruta indicada`.
    Causa probable: ruta invalida o archivo inexistente.
    Solucion: usar `load` y presionar Enter para cargar el catalogo por defecto, o ingresar ruta valida.

- Error en `new` indicando que una cancion no existe en catalogo.
    Causa probable: la cancion escrita no esta en el catalogo activo.
    Solucion: ejecutar `catalog` para ver nombres exactos y repetir `new` con canciones validas.

- `songs <lista>` indica que la lista no existe.
    Causa probable: nombre incorrecto o lista no cargada en la coleccion activa.
    Solucion: ejecutar `lists` para confirmar nombres y volver a intentar.

- El comando no es reconocido.
    Causa probable: sintaxis incorrecta.
    Solucion: usar solo comandos soportados: `play`, `current`, `catalog`, `lists`, `songs`, `new`, `load`, `q`.
