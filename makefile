all: main.cpp loader.h geom.h scenegraph.h nodes.h
	g++ -std=c++11 -o main main.cpp -lGL -lGLU -lglut -L./src/lib -lglui

clean:
	rm -f main
