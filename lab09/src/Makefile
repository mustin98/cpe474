CC=g++
CFLAGS=-ansi -pedantic -Wno-deprecated
INC=-I$(EIGEN3_INCLUDE_DIR) -I$(CERES_DIR)/build/include -I$(CERES_DIR)/internal/ceres/miniglog
LIB=-L$(CERES_DIR)/build/lib -lceres

all:
	$(CC) $(CFLAGS) $(INC) *.cpp $(LIB)

clean:
	rm -f *~ *.o a.out
