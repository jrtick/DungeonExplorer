all: dungeon

CC = g++ -std=c++11
DEBUG = -g -Wall
IMAGE_LIBS = -ljpeg -lpng
OPENGL_LIBS = -lGL -lGLU -lglut
OPENAL_LIBS = -lopenal -lalut

Image.o: ImageLoader.cpp Texture.cpp
	$(CC) $(DEBUG) -c ImageLoader.cpp -o Image.o

Sound.o: Sounds.cpp
	$(CC) $(DEBUG) -c Sounds.cpp -o Sound.o

dungeon: Image.o Sound.o dungeon.cpp
	$(CC) $(DEBUG) -o dungeon.exe Image.o Sound.o dungeon.cpp Texture.cpp $(OPENGL_LIBS) $(IMAGE_LIBS) $(OPENAL_LIBS)

#dash won't let its output halt execution
#oring with true won't count is as a failure
clean:
	-rm dungeon.exe dungeon.tar *.o || true

tar:
	tar cfv dungeon.tar dungeon.cpp Makefile dungeon
