CC=g++
CFLAGS=-ansi -pedantic -Wno-deprecated -O3
INC=-I$(EIGEN3_INCLUDE_DIR)
LIB=-DGL_GLEXT_PROTOTYPES -lglut -lGL -lGLU
LIB_OSX=-framework GLUT -framework OpenGL

all:
	$(CC) $(CFLAGS) $(INC) *.cpp *.cc $(LIB)

osx:
	$(CC) $(CFLAGS) $(INC) *.cpp *.cc $(LIB_OSX)

clean:
	rm -f *~ *.o a.out
