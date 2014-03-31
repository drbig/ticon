CFLAGS=-Wall -O2
CFLAGS+=`pkg-config --cflags lua gtk+-2.0`
LDFLAGS+=`pkg-config --libs lua gtk+-2.0`

all: ticon

debug: CFLAGS+=-DDEBUG
debug: all

clean:
	rm -f ticon
