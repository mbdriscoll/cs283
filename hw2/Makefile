TARGET = trace
OBJECTS = image.o scene.o

CFLAGS = -I/opt/local/include -I. -g
CXXFLAGS = -I/opt/local/include -I. -g
LDFLAGS = -L/opt/local/lib -lpng

default: $(TARGET)

$(TARGET): $(OBJECTS)

image.cpp: image.h

.PHONY: clean
clean:
	rm -rf $(TARGET) $(OBJECTS)
