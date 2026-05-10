TARGET = build/reproductor.exe
CC = gcc

CFLAGS = -Wall -Wextra -std=c11 -O0 -g -Iinclude

SRC_DIR = src
BUILD_DIR = build

SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC))

all: check_toolchain $(BUILD_DIR) $(TARGET)

check_toolchain:
	@for /f "delims=" %%i in ('$(CC) --print-prog-name=cc1.exe') do @if /i "%%i"=="cc1.exe" (echo Error: tu instalacion de GCC en MinGW parece incompleta. & echo Instala un toolchain completo o usa MSYS2/MinGW-w64 antes de compilar. & exit /b 1)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)

clean:
	if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)
	if exist $(TARGET) del $(TARGET)

.PHONY: all clean check_toolchain