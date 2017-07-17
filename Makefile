all: dungeon

CC = g++
DEBUG = -g -Wall
IMAGE_LIBS = -ljpeg -lpng
OPENGL_LIBS = -lGL -lGLU -lglut

Image.o: ImageLoader.cpp
	$(CC) $(DEBUG) -c ImageLoader.cpp -o Image.o

dungeon: Image.o dungeon.cpp
	$(CC) $(DEBUG) -o dungeon.exe Image.o dungeon.cpp $(OPENGL_LIBS) $(IMAGE_LIBS)

#dash won't let its output halt execution
#oring with true won't count is as a failure
clean:
	-rm dungeon.exe dungeon.tar *.o || true

tar:
	tar cfv dungeon.tar dungeon.cpp Makefile dungeon