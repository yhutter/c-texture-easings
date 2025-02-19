build:
	clang -Wpedantic -Wall -O3 -std=c99 ./src/*.c `pkg-config sdl3 --cflags --libs` -o texture_easings
run:
	./texture_easings
