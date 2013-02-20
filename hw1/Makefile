CXX = c++
CC = c++

OBJECTS = \
		  Object.o \
		  extra/hud.o \
		  extra/gl_hud.o \
		  viewer.o

CXXFLAGS = -I/opt/local/include -O2
LDFLAGS = -framework GLUT -framework OpenGL
default: viewer

viewer: $(OBJECTS)

clean:
	rm -rf viewer $(OBJECTS)
