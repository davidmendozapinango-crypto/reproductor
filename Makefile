
TARGET_CLI = build/reproductor.exe
TARGET_UX = build/reproductor-ux.exe
CC = gcc

CFLAGS = -Wall -Wextra -std=c11 -O0 -g -Iinclude
NCURSES_FLAGS = -lncurses
NCURSESW_FLAGS = -lncursesw
PDCURSES_FLAGS = -lpdcurses
UX_INC_FLAGS = -IC:/msys64/mingw64/include -IC:/msys64/mingw64/include/ncurses -IC:/msys64/mingw64/include/ncursesw
UX_LIB_FLAGS = -LC:/msys64/mingw64/lib

SRC_DIR = src
BUILD_DIR = build

SRC = $(wildcard $(SRC_DIR)/*.c)
SRC_CLI = $(filter-out $(SRC_DIR)/ui.c,$(SRC))
SRC_UX = $(SRC)

all: check_toolchain $(BUILD_DIR) $(TARGET_CLI) $(TARGET_UX)

check_toolchain:
	@for /f "delims=" %%i in ('$(CC) --print-prog-name=cc1.exe') do @if /i "%%i"=="cc1.exe" (echo Error: tu instalacion de GCC en MinGW parece incompleta. & echo Instala un toolchain completo o usa MSYS2/MinGW-w64 antes de compilar. & exit /b 1)



$(TARGET_CLI): $(SRC_CLI)
	$(CC) $(CFLAGS) $^ -o $@

$(TARGET_UX): $(SRC_UX)
	$(CC) $(CFLAGS) $(UX_INC_FLAGS) -DREPRODUCTOR_UX $^ -o $@ $(UX_LIB_FLAGS) $(NCURSESW_FLAGS) || $(CC) $(CFLAGS) $(UX_INC_FLAGS) -DREPRODUCTOR_UX $^ -o $@ $(UX_LIB_FLAGS) $(NCURSES_FLAGS) || $(CC) $(CFLAGS) $(UX_INC_FLAGS) -DREPRODUCTOR_UX $^ -o $@ $(UX_LIB_FLAGS) $(PDCURSES_FLAGS)

$(BUILD_DIR):
	if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)

clean:
	if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)
	if exist $(TARGET_CLI) del $(TARGET_CLI)
	if exist $(TARGET_UX) del $(TARGET_UX)

.PHONY: all clean check_toolchain