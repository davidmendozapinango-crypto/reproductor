TARGET_CLI = build/reproductor
TARGET_UX = build/reproductor-ux
CC = gcc

CFLAGS = -Wall -Wextra -std=c11 -O0 -g -Iinclude -I/usr/include -I/usr/include/ncurses
NCURSES_FLAGS = -lncurses -lncursesw

SRC_DIR = src
BUILD_DIR = build

SRC = $(wildcard $(SRC_DIR)/*.c)
SRC_CLI = $(filter-out $(SRC_DIR)/ui.c,$(SRC))
SRC_UX = $(SRC)

all: $(BUILD_DIR) $(TARGET_CLI) $(TARGET_UX)

$(TARGET_CLI): $(SRC_CLI)
	$(CC) $(CFLAGS) $^ -o $@

$(TARGET_UX): $(SRC_UX)
	$(CC) $(CFLAGS) -DREPRODUCTOR_UX $^ -o $@ $(NCURSES_FLAGS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	if [ -d $(BUILD_DIR) ]; then rm -rf $(BUILD_DIR); fi
	if [ -f $(TARGET_CLI) ]; then rm -f $(TARGET_CLI); fi
	if [ -f $(TARGET_UX) ]; then rm -f $(TARGET_UX); fi