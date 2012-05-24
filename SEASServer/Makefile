# Makefile for SEASServer Daemon
# By: Michael Lindemuth

CC=gcc
LD=g++
CSRCS = $(wildcard *.c) 
CPPSRCS += $(wildcard *.cpp)
COBJS = $(patsubst %.c,objs/%.o,$(CSRCS))
COBJS += $(wildcard MethodParser/*.o)
CPPOBJS = $(patsubst %.cpp,objs/%.o,$(CPPSRCS))

INC_DIRS = -I../USB4000Gum/include \
           -IMethodParser
LDFLAGS = -L../USB4000Gum/lib
LIBS = -lUSB4000 -lsqlite3 -lusb -lm -lpthread


SEASServer:$(COBJS) $(CPPOBJS)
	$(LD) -o $@ $(INC_DIRS) $(LDFLAGS) $(LIBS) $^; \
	sudo cp SEASServer /usr/local/bin;

objs/%.o:%.c 
	$(CC) -O2 -c $(CFLAGS) $(INC_DIRS) $< -o $@; 

objs/%.o:%.cpp
	g++ -O2 -c $(CFLAGS) $(INC_DIRS) $< -o $@;

MethodParser/%.o:MethodParser/%.c 
	$(CC) -O2 -c $(CFLAGS) $(INC_DIRS) $< -o $@;

MethodParser/MethodLexAna.lex.c: MethodParser/MethodParser.tab.c MethodParser/MethodLexAna.y
	flex -o $@ -i MethodParser/MethodLexAna.y 

MethodParser/MethodParser.tab.c: MethodParser/MethodParser.y
	bison -d MethodParser/MethodParser.y -o $@

