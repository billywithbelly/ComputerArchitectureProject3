CC = g++ -lm -std=c++11

SRCS = ./*.cpp
OBS = ./*.o

# The following line means that do SRC first.
pipeline: SRC
	$(CC) -o $@ *.o

SRC: $(SRCS)
	$(CC) -c $(SRCS)
	
.PHONY :
	clean
clean: 
	rm pipeline *.o