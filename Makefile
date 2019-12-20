CFLAGS = -g3 -O3 -flto -funroll-all-loops
all: chip9
chip9: chip9.c
	$(CC) $(CFLAGS)  chip9.c -o chip9 -lSDL
