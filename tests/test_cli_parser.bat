@echo off
setlocal

set "OUT_FILE=build\cli_parser_output.txt"
set "IN_FILE=build\cli_parser_input.txt"
set "APP_EXE=build\reproductor_cli_test.exe"

gcc -Wall -Wextra -std=c11 -O0 -g -Iinclude src\main.c src\cancion.c src\catalogo.c src\historial.c src\utils.c -o "%APP_EXE%"
if errorlevel 1 exit /b 1

(
    echo.
    echo back
    echo play "The Search"
    echo queue "Back in Black"
    echo shuffle
    echo shuffle
    echo next
    echo current
    echo back
    echo current
    echo q
) > "%IN_FILE%"

"%APP_EXE%" < "%IN_FILE%" > "%OUT_FILE%" 2>&1
if errorlevel 1 exit /b 1

findstr /C:"No hay canciones en el historial de reproduccion." "%OUT_FILE%" >nul
if errorlevel 1 (
    echo FALLO: back no reporta historial vacio cuando corresponde.
    type "%OUT_FILE%"
    exit /b 1
)

findstr /C:"Shuffle activado. Cola en espera mezclada." "%OUT_FILE%" >nul
if errorlevel 1 (
    echo FALLO: shuffle no se activo correctamente.
    type "%OUT_FILE%"
    exit /b 1
)

findstr /C:"Shuffle desactivado. Se mantiene el orden actual." "%OUT_FILE%" >nul
if errorlevel 1 (
    echo FALLO: shuffle no se desactivo correctamente.
    type "%OUT_FILE%"
    exit /b 1
)

findstr /C:"Actual: Back in Black" "%OUT_FILE%" >nul
if errorlevel 1 (
    echo FALLO: current no muestra la cancion esperada tras next.
    type "%OUT_FILE%"
    exit /b 1
)

findstr /C:"Actual: The Search" "%OUT_FILE%" >nul
if errorlevel 1 (
    echo FALLO: current no muestra la cancion esperada tras back.
    type "%OUT_FILE%"
    exit /b 1
)

echo Tests passed: %APP_EXE% ^(flujo parser interactivo^)
exit /b 0
