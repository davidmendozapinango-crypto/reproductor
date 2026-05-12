@echo off
setlocal

gcc -Wall -Wextra -std=c11 -O0 -g -Iinclude tests\test_utils.c src\cancion.c src\catalogo.c src\historial.c src\utils.c -o build\test_utils.exe
if errorlevel 1 exit /b 1

build\test_utils.exe
if errorlevel 1 exit /b 1

call tests\test_cli_parser.bat
if errorlevel 1 exit /b 1

set "UX_GCC=gcc"
if exist C:\msys64\mingw64\bin\gcc.exe (
	set "UX_GCC=C:\msys64\mingw64\bin\gcc.exe"
	set "PATH=C:\msys64\mingw64\bin;C:\msys64\usr\bin;%PATH%"
)

"%UX_GCC%" -Wall -Wextra -std=c11 -O0 -g -Iinclude -IC:\msys64\mingw64\include -IC:\msys64\mingw64\include\ncurses -IC:\msys64\mingw64\include\ncursesw ^
tests\test_ui_commands.c src\ui.c src\cancion.c src\catalogo.c src\historial.c src\utils.c ^
-o build\test_ui_commands.exe -LC:\msys64\mingw64\lib -lncursesw
if errorlevel 1 exit /b 1

build\test_ui_commands.exe
if errorlevel 1 exit /b 1

echo Tests passed: build\test_utils.exe
