# Makefile for Method Parser Test Program
# By: Michael Lindemuth

# Build the Server

SEASServer: libUSB4000 MethodParser 
	cd SEASServer; \
	gcc -I../USB4000Gum/include -L../USB4000Gum/lib -lUSB4000 -lusb -lm -lpthread -IMethodParser MethodParser/*.c *.c -o SEASServer
	sudo cp SEASServer/SEASServer /usr/local/bin; \

MethodParser:
	cd SEASServer/MethodParser; \
	make; \

libUSB4000:
	cd USB4000Gum; \
	make; \

