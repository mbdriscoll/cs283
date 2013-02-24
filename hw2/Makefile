TARGET = trace
OBJECTS = trace.o image.o

CFLAGS = -I/opt/local/include
LDFLAGS = -L/opt/local/lib -lpng

default: $(TARGET)

$(TARGET): $(OBJECTS)

image.o: image.c image.h

.PHONY: clean
clean:
	rm -rf $(TARGET) $(OBJECTS)
