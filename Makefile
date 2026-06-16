CC = gcc
CFLAGS = -Wall -Wextra -g
SRCS = ROUTEMAPPER.c tdas/extra.c tdas/heap.c tdas/list.c tdas/map.c
TARGET = ROUTEMAPPER

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) -lm

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
