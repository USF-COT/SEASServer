# Makefile for SEASServer Daemon
# By: Michael Lindemuth

CC=gcc
LD=$(CC)
SRCS = $(wildcard *.c) 
OBJS = $(patsubst %.c,objs/%.o,$(SRCS))
OBJS += $(wildcard MethodParser/*.o)

INC_DIRS = -I../USB4000Gum/include \
           -IMethodParser
LDFLAGS = -L../USB4000Gum/lib
LIBS = -lUSB4000 -lsqlite3 -lusb -lm -lpthread


SEASServer:$(OBJS)
	$(LD) -o $@ $(INC_DIRS) $(LDFLAGS) $(LIBS) $^; \
	sudo cp SEASServer /usr/local/bin;

objs/%.o:%.c
	$(CC) -c $(CFLAGS) $(INC_DIRS) $< -o $@; 
