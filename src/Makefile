PKG_CONFIG = gtk+-2.0 gthread-2.0
SOURCES = main.c \
		  callbacks.c callbacks.h \
		  error.c error.h \
		  stream.c stream.h \
		  window.c window.h \
		  glk.c glk.h \
		  gestalt.c \
		  fileref.c fileref.h \
		  case.c \
		  strio.c \
		  event.c event.h \
		  input.c input.h \
		  style.c \
		  first.c
OBJECTS = main.o \
		  callbacks.o \
		  error.o \
		  stream.o \
		  window.o \
		  glk.o \
		  gestalt.o \
		  fileref.o \
		  case.o \
		  strio.o \
		  event.o \
		  input.o \
		  style.o \
		  first.o

CFLAGS = -g -Wall -O0 -export-dynamic `pkg-config --cflags ${PKG_CONFIG}`
LIBS = -export-dynamic `pkg-config --libs ${PKG_CONFIG}`

all: gargoyle-gtk gargoyle-gtk.ui

gargoyle-gtk: ${OBJECTS}
	${CC} ${LIBS} -o gargoyle-gtk ${OBJECTS}

gargoyle-gtk.ui: gargoyle-gtk.glade
	gtk-builder-convert gargoyle-gtk.glade gargoyle-gtk.ui
	
clean:
	${RM} -f ${OBJECTS} gargoyle-gtk gargoyle-gtk.ui
