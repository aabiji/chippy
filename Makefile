all:
	gcc main.c -o bin/chippy -O1 -O3 -Wall -Wno-missing-braces -I external/include/ -L external/lib -lraylib -lopengl32 -lgdi32 -lwinmm
