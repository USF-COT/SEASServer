# Makefile for USB4000 Spectrometer Driver
#
# By: Michael Lindemuth
# University of South Florida
# College of Marine Science
# Center for Ocean Technology

install: lib/libUSB4000.so
	sudo cp lib/libUSB4000.so /lib/; \
	sudo ldconfig; \

testUSB4000: lib/libUSB4000.so
	gcc -Iinclude -lusb testUSB4000.c -o testUSB4000 -Llib -lUSB4000

lib/libUSB4000.so: USB4000.o
	gcc -shared -o lib/libUSB4000.so USB4000.o

USB4000.o: USB4000Gum.c USB4000Gum.h include/libUSB4000.h
	gcc -lusb -lm -c -fPIC USB4000Gum.c -o USB4000.o
