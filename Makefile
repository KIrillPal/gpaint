all: obj/Image.o obj/BMP.o obj/Negative.o obj/ReplaceColor.o src/main.cpp
	g++ obj/Image.o obj/BMP.o obj/Negative.o obj/ReplaceColor.o src/main.cpp -o gpaint
obj/ReplaceColor.o: FilterHeaders src/Filters/gpaint_filters.h src/Filters/ReplaceColor.cpp
	g++ obj/Image.o -c src/Filters/ReplaceColor.cpp -o obj/ReplaceColor.o
obj/Negative.o: FilterHeaders src/Filters/Negative.cpp
	g++ obj/Image.o -c src/Filters/Negative.cpp -o obj/Negative.o
FilterHeaders: obj/Image.o src/Filters/ImageFilter.h src/Filters/gpaint_filters.h
obj/BMP.o: obj/Image.o src/BMP.h src/BMP.cpp
	g++ obj/Image.o -c src/BMP.cpp -o obj/BMP.o
obj/Image.o: src/gpaint_exception.h src/Image.h src/Image.cpp
	g++ -c src/Image.cpp -o obj/Image.o
run: all
	./gpaint
clear:
	rm obj/*
