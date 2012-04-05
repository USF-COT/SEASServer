# Makefile for Method Parser Test Program
# By: Michael Lindemuth

# Build the Server

SEASServer: libUSB4000 
	cd SEASServer; \
	make; \

libUSB4000:
	cd USB4000Gum; \
	make; \

