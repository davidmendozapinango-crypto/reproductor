@echo off
setlocal

if not exist build mkdir build

for /f "delims=" %%i in ('gcc --print-prog-name=cc1.exe') do set "CC1=%%i"
if /i "%CC1%"=="cc1.exe" (
	echo Error: tu instalacion de GCC en MinGW parece incompleta.
	echo Instala un toolchain completo o usa MSYS2/MinGW-w64 antes de compilar.
	exit /b 1
)

gcc -Wall -Wextra -std=c11 -O0 -g -Iinclude src\main.c src\utils.c src\cancion.c -o build\reproductor.exe
if errorlevel 1 exit /b 1

echo Build completed: build\reproductor.exe
