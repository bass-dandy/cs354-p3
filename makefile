all: main.cpp loader.h geom.h
	g++ -std=c++11 -o main main.cpp -lGL -lGLU -lglut

clean:
	rm -f main
