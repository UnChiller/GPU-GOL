all: build

run: build
	@cd bin;./gpugol

build:
	@gcc -o bin/gpugol src/main.c -lglfw -lGLEW -lGL -Wall -Wextra -O3
	@cp -r src/shaders/* bin/

clean:
	@rm -r $(TARGET) $(SHADER_OBJS)

.PHONY: all clean run build