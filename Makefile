CXXFLAGS = -O2 
VERSION = final
all : sokoban_$(VERSION)

sokoban_$(VERSION) : board.o settings.o objects.o main.o
	g++ $(CXXFLAGS) -o sokoban_$(VERSION) board.o settings.o objects.o main.o -lncurses -lform

board.o : board.cpp board.h objects.h
	g++ $(CXXFLAGS) -c board.cpp -o board.o

settings.o : settings.cpp board.h objects.h settings.h
	g++ $(CXXFLAGS) -c settings.cpp -o settings.o

objects.o : objects.cpp board.h objects.h
	g++ $(CXXFLAGS) -c objects.cpp -o objects.o

main.o : main.cpp board.h objects.h settings.h
	g++ $(CXXFLAGS) -c main.cpp -o main.o

clean :
	rm -f sokoban_$(VERSION) *.o *~ *#
