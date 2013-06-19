# Makefile for SEASServer Daemon
# By: Michael Lindemuth

CC=gcc
LD=g++
CSRCS = $(wildcard *.c) 
CPPSRCS += $(wildcard *.cpp)
METHODSRCS = $(wildcard MethodParser/Method*.c)
COBJS = $(patsubst %.c,objs/%.o,$(CSRCS))
CPPOBJS = $(patsubst %.cpp,objs/%.o,$(CPPSRCS))
METHODOBJS = $(patsubst MethodParser/%.c,MethodParser/%.o,$(METHODSRCS))

INC_DIRS = -I../USB4000Gum/include \
           -IMethodParser
LDFLAGS = -L../USB4000Gum/lib -L/lib
LIBS = -lUSB4000 -lsqlite3 -lusb -lm -lpthread 


SEASServer:$(COBJS) $(CPPOBJS) $(METHODOBJS)
	$(LD) -O2 -o $@ $(INC_DIRS) $(LDFLAGS) $(LIBS) $^; \
	sudo cp SEASServer /usr/local/bin;

objs/%.o:%.c | objs_dir
	$(CC) -O2  -c $(CFLAGS) $(INC_DIRS) $< -o $@; 

objs/%.o:%.cpp | objs_dir
	g++ -O2 -c $(CFLAGS) $(INC_DIRS) $< -o $@;

objs_dir:
	mkdir -p objs;

MethodParser/%.o:MethodParser/%.c 
	$(CC) -O2  -c $(CFLAGS) $(INC_DIRS) $< -o $@;

MethodParser/MethodLexAna.lex.c: MethodParser/MethodParser.tab.c MethodParser/MethodLexAna.y
	flex -o $@ -i MethodParser/MethodLexAna.y 

MethodParser/MethodParser.tab.c: MethodParser/MethodParser.y
	bison -d MethodParser/MethodParser.y -o $@

