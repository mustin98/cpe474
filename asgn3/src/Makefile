CC=g++
CFLAGS=-ansi -pedantic -Wno-deprecated -g -o a3
INC=-I$(EIGEN3_INCLUDE_DIR) -I$(EIGEN3_INCLUDE_DIR) -I$(CERES_DIR)/build/include -I$(CERES_DIR)/internal/ceres/miniglog
LIB=-DGL_GLEXT_PROTOTYPES -lglut -lGL -lGLU -L$(CERES_DIR)/build/lib -lceres

all:
	$(CC) $(CFLAGS) $(INC) *.cpp *.cc $(LIB)

clean:
	rm -f *~ *.o a.out
