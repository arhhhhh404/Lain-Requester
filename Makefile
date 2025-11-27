TARGET = lain
CC = gcc

CFLAGS = -pedantic -Wall -Wextra -Iinclude
LIBS = -lSDL2 -lSDL2_ttf -lSDL2_image -lcurl

SRCS = requester.c include/request_parser.c include/request_response.c include/darknet_launcher.c

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LIBS)

clean:
	rm -f $(OBJS) $(TARGET)
