TARGET = trace
OBJECTS = image.o scene.o preview.o

CFLAGS = -I/opt/local/include -I. -g
CXXFLAGS = -I/opt/local/include -I. -g
LDFLAGS = -L/opt/local/lib -lpng -framework GLUT -framework OpenGL

default: $(TARGET)

$(TARGET): $(OBJECTS)

image.cpp: image.h
scene.cpp: scene.h

.PHONY: clean
clean:
	rm -rf $(TARGET) $(OBJECTS) *.png
