@echo off
setlocal

if not exist build mkdir build

for /f "delims=" %%i in ('gcc --print-prog-name=cc1.exe') do set "CC1=%%i"
if /i "%CC1%"=="cc1.exe" (
	echo Error: tu instalacion de GCC en MinGW parece incompleta.
	echo Instala un toolchain completo o usa MSYS2/MinGW-w64 antes de compilar.
	exit /b 1
)

set "UX_GCC=gcc"
if exist C:\msys64\mingw64\bin\gcc.exe (
	set "UX_GCC=C:\msys64\mingw64\bin\gcc.exe"
	set "PATH=C:\msys64\mingw64\bin;C:\msys64\usr\bin;%PATH%"
)

gcc -Wall -Wextra -std=c11 -O0 -g -Iinclude ^
src\main.c src\utils.c src\cancion.c src\catalogo.c src\historial.c ^
-o build\reproductor.exe
if errorlevel 1 exit /b 1

REM Compilar la version UX con ncurses
"%UX_GCC%" -Wall -Wextra -std=c11 -O0 -g -Iinclude -IC:\msys64\mingw64\include -IC:\msys64\mingw64\include\ncurses -IC:\msys64\mingw64\include\ncursesw -DREPRODUCTOR_UX ^
src\main.c src\utils.c src\cancion.c src\catalogo.c src\historial.c src\ui.c ^
-o build\reproductor-ux.exe -LC:\msys64\mingw64\lib -lncursesw
if errorlevel 1 (
	"%UX_GCC%" -Wall -Wextra -std=c11 -O0 -g -Iinclude -IC:\msys64\mingw64\include -IC:\msys64\mingw64\include\ncurses -IC:\msys64\mingw64\include\ncursesw -DREPRODUCTOR_UX ^
	src\main.c src\utils.c src\cancion.c src\catalogo.c src\historial.c src\ui.c ^
	-o build\reproductor-ux.exe -LC:\msys64\mingw64\lib -lncurses
	if errorlevel 1 (
		"%UX_GCC%" -Wall -Wextra -std=c11 -O0 -g -Iinclude -IC:\msys64\mingw64\include -IC:\msys64\mingw64\include\ncurses -IC:\msys64\mingw64\include\ncursesw -DREPRODUCTOR_UX ^
		src\main.c src\utils.c src\cancion.c src\catalogo.c src\historial.c src\ui.c ^
		-o build\reproductor-ux.exe -LC:\msys64\mingw64\lib -lpdcurses
		if errorlevel 1 (
			echo Error: no se pudo compilar build\reproductor-ux.exe con ncursesw, ncurses ni pdcurses.
			echo Sugerencia: instala mingw-w64-x86_64-ncurses o pdcurses en tu toolchain.
			exit /b 1
		)
	)
)

echo Build completed: build\reproductor.exe y build\reproductor-ux.exe
