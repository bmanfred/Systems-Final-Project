CC=		gcc
CFLAGS=		-g -Wall -std=gnu99 -Iinclude
LD=		gcc
LDFLAGS=	-L.
AR=		ar
ARFLAGS=	rcs
TARGETS=	bin/spidey
HEADER=         include/spidey.h

all:		$(TARGETS)

clean:
	@echo Cleaning...
	@rm -f $(TARGETS) lib/*.a src/*.o *.log *.input

.PHONY:		all test clean

# TODO: Add rules for bin/spidey, lib/libspidey.a, and any intermediate objects

# Build object recipes

# Spidey object
src/spidey.o:		src/spidey.c $(HEADER)
	@echo Compiling src/spidey.o...
	-@ $(CC) $(CFLAGS) -c -o $@ $<

# Library objects
src/forking.o:		src/forking.c $(HEADER)
	@echo Compiling src/forking.o...
	-@ $(CC) $(CFLAGS) -c -o $@ $<
        
src/handler.o:		src/handler.c $(HEADER)
	@echo Compiling src/handler.o...
	-@ $(CC) $(CFLAGS) -c -o $@ $<

src/request.o: 		src/request.c $(HEADER)
	@echo Compiling src/request.o...
	-@ $(CC) $(CFLAGS) -c -o $@ $<

src/single.o: 		src/single.c $(HEADER)
	@echo Compiling src/single.o...
	-@ $(CC) $(CFLAGS) -c -o $@ $<

src/socket.o: 		src/socket.c $(HEADER)
	@echo Compiling src/socket.o...
	-@ $(CC) $(CFLAGS) -c -o $@ $<

src/utils.o: 		src/utils.c $(HEADER)
	@echo Compiling src/utils.o...
	-@ $(CC) $(CFLAGS) -c -o $@ $<

# Linking recipes

# Spidey static library 
lib/libspidey.a:	src/forking.o src/handler.o src/request.o src/single.o src/socket.o src/utils.o
	@echo Linking libspidey.a...
	-@ $(AR) $(ARFLAGS) $@ $^

# Spidey executable
bin/spidey:		src/spidey.o lib/libspidey.a
	@echo Linking bin/spidey...
	-@ $(LD) $(LDFLAGS) -o $@ $^

