CC = gcc
CFLAGS = -Wall 
SRCS = webServer.c http_message.c
OBJS = webServer.o http_message.o
TARGET = webServer

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

%.o: %.c http_message.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) 
