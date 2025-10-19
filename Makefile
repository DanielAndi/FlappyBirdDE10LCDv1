CC = gcc
CFLAGS = -O2 -Wall -std=c99
TARGET = flappy_bird
SOURCES = main.c game_state.c
HEADERS = game_state.h address_map_arm.h

$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

clean:
	rm -f $(TARGET)

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

.PHONY: clean install