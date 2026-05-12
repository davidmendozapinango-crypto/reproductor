@echo off
setlocal

gcc -Wall -Wextra -std=c11 -O0 -g -Iinclude tests\test_utils.c src\cancion.c src\utils.c -o build\test_utils.exe
if errorlevel 1 exit /b 1

build\test_utils.exe
if errorlevel 1 exit /b 1

call tests\test_cli_parser.bat
if errorlevel 1 exit /b 1

echo Tests passed: build\test_utils.exe
