CFLAGS = -Werror -Wall -Wno-unused
CFLAGS += -g  # add debug symbols
LIBS   = -lX11 -lGL -lpthread

SRC_FILES =                  \
	../olcPGE_min_x11_gdi.c  \
	test1.c

all:

	gcc $(CFLAGS) $(SRC_FILES) $(LIBS) -o bin/test.e
