all: obj/Image.o obj/BMP.o Filters src/main.cpp
	g++ obj/*.o src/main.cpp -o gpaint
Filters: obj/Negative.o obj/ReplaceColor.o obj/Convolution.o obj/Gauss.o
FilterHeaders: obj/Image.o src/Filters/ImageFilter.h src/Filters/gpaint_filters.h

obj/Negative.o:     FilterHeaders src/Filters/Negative.cpp
	g++ obj/Image.o -c src/Filters/Negative.cpp -o obj/Negative.o
obj/ReplaceColor.o: FilterHeaders src/Filters/ReplaceColor.cpp
	g++ obj/Image.o -c src/Filters/ReplaceColor.cpp -o obj/ReplaceColor.o
obj/Convolution.o:  FilterHeaders src/Filters/Convolution.cpp
	g++ obj/Image.o -c src/Filters/Convolution.cpp -o obj/Convolution.o
obj/Gauss.o:      	FilterHeaders src/Filters/Gauss.cpp
	g++ obj/Image.o -c src/Filters/Gauss.cpp -o obj/Gauss.o

obj/BMP.o: obj/Image.o src/BMP.h src/BMP.cpp
	g++ obj/Image.o -c src/BMP.cpp -o obj/BMP.o
obj/Image.o: src/gpaint_exception.h src/Image.h src/Image.cpp
	g++ -c src/Image.cpp -o obj/Image.o
run: all
	./gpaint
clear:
	rm obj/*
