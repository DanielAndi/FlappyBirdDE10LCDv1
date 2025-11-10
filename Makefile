CC = gcc
CFLAGS = -O2 -Wall -std=c99 -D_DEFAULT_SOURCE -I. -Ihardware -Idisplay -Igame -Iinput -Iassets
TARGET = flappy_bird

SOURCES = \
	main.c \
	hardware/hps_fpga_bridge.c \
	hardware/fpga_peripherals.c \
	display/lcd_driver.c \
	display/graphics.c \
	display/sprites.c \
	game/game_state.c \
	game/physics.c \
	game/collision.c \
	game/game_objects.c \
	game/gameplay.c \
	input/input_handler.c

TEST_TARGET = hardware/tests/run_hardware_tests
TEST_SOURCES = \
	hardware/tests/hardware_tests.c \
	hardware/hps_fpga_bridge.c \
	hardware/fpga_peripherals.c

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

$(TEST_TARGET): $(TEST_SOURCES)
	$(CC) $(CFLAGS) $(TEST_SOURCES) -o $(TEST_TARGET)

hardware-tests: $(TEST_TARGET)

test-hardware: hardware-tests
	./$(TEST_TARGET)

clean:
	rm -f $(TARGET) $(TEST_TARGET)

.PHONY: clean hardware-tests test-hardware