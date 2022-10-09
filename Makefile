all: obj/Image.o obj/BMP.o src/main.cpp
	g++ obj/Image.o obj/BMP.o src/main.cpp -o gpaint
BMP.o: obj/Image.o obj/BMP.h src/BMP.cpp
	g++ obj/Image.o -c src/BMP.cpp -o obj/BMP.o
Image.o: src/Image.h src/Image.cpp
	g++ -c src/Image.cpp -o obj/Image.o
run: all
	./gpaint
clear:
	rm obj/*
