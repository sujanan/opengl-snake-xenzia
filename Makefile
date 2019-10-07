CC=gcc
LIBS=-lGLU -lGL -lglut -lm

snake: snake.o
	$(CC) -o snake snake.o $(LIBS)

clean:
	rm -rf *.o snake

