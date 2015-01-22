all:
	g++ -ansi -pedantic -Wno-deprecated *.cpp -DGL_GLEXT_PROTOTYPES -lglut -lGL -lGLU

osx:
	g++ -ansi -pedantic -Wno-deprecated *.cpp -framework GLUT -framework OpenGL

clean:
	rm -f *~ *.o a.out
