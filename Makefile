.PHONY: all clean

all: obj obj/Image.o obj/BMP.o Filters obj/TerminalParser.o obj/Helper.o src/main.cpp
	g++ obj/Image.o obj/BMP.o obj/TerminalParser.o obj/Helper.o obj/filters.o src/main.cpp -o gpaint
Filters:  obj/Negative.o obj/ReplaceColor.o obj/Convolution.o obj/Gauss.o obj/Gray.o obj/Median.o obj/Sobel.o obj/Crop.o obj/Resize.o
	ld -r obj/Negative.o obj/ReplaceColor.o obj/Convolution.o obj/Gauss.o obj/Gray.o obj/Sobel.o obj/Median.o obj/Crop.o obj/Resize.o -o obj/filters.o
FilterHeaders: src/Filters/ImageFilter.h src/Filters/gpaint_filters.h

obj/Negative.o:     FilterHeaders src/Filters/Negative.cpp
	g++ -c src/Filters/Negative.cpp -o obj/Negative.o
obj/ReplaceColor.o: FilterHeaders src/Filters/ReplaceColor.cpp
	g++ -c src/Filters/ReplaceColor.cpp -o obj/ReplaceColor.o
obj/Convolution.o:  FilterHeaders src/Filters/Convolution.cpp
	g++ -c src/Filters/Convolution.cpp -o obj/Convolution.o
obj/Gauss.o:      	FilterHeaders src/Filters/Gauss.cpp
	g++ -c src/Filters/Gauss.cpp -o obj/Gauss.o
obj/Gray.o:      	FilterHeaders src/Filters/Gray.cpp
	g++ -c src/Filters/Gray.cpp -o obj/Gray.o
obj/Sobel.o:      	FilterHeaders src/Filters/Sobel.cpp
	g++ -c src/Filters/Sobel.cpp -o obj/Sobel.o
obj/Median.o:      	FilterHeaders src/Filters/Median.cpp
	g++ -c src/Filters/Median.cpp -o obj/Median.o
obj/Crop.o:      	FilterHeaders src/Filters/Crop.cpp
	g++ -c src/Filters/Crop.cpp -o obj/Crop.o
obj/Resize.o:      	FilterHeaders src/Filters/Resize.cpp
	g++ -c src/Filters/Resize.cpp -o obj/Resize.o

obj/Helper.o: src/TerminalParser.h src/Helper.cpp
	g++ -c src/Helper.cpp -o obj/Helper.o
obj/TerminalParser.o: Filters src/TerminalParser.cpp src/TerminalParser.h
	g++ -c src/TerminalParser.cpp -o obj/TerminalParser.o
obj/BMP.o: obj/Image.o src/BMP.h src/BMP.cpp
	g++ -c src/BMP.cpp -o obj/BMP.o
obj/Image.o: src/gpaint_exception.h src/Image.h src/Image.cpp
	g++ -c src/Image.cpp -o obj/Image.o
run: all
	./gpaint
obj:
	mkdir obj
clean:
	rm obj/*
	rmdir obj
