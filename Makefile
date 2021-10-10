EXENAME = CityGenerator
OBJS = main.o City.o HouseSet.o PNG.o HSLAPixel.o lodepng.o

CXX = clang++
CXXFLAGS = -std=c++1y -stdlib=libc++ -c -g -O0 -Wall -Wextra -pedantic
LD = clang++
LDFLAGS = -std=c++1y -stdlib=libc++ -lc++abi -lm

.PHONY: clean output_msg

all : $(EXENAME)

$(EXENAME) : output_msg $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) -o $(EXENAME)

# main:
	
main.o : main.cpp City.h HouseSet.h PNGimage/PNG.h PNGimage/HSLAPixel.h
	$(CXX) $(CXXFLAGS) main.cpp
 
# CityGenerator:

City.o: City.cpp City.h HouseSet.h PNGimage/PNG.h 
	$(CXX) $(CXXFLAGS) City.cpp
 
HouseSet.o: HouseSet.cpp HouseSet.h
	$(CXX) $(CXXFLAGS) HouseSet.cpp

# PNGimage:

PNG.o : PNGimage/PNG.cpp PNGimage/PNG.h PNGimage/HSLAPixel.h PNGimage/lodepng/lodepng.h
	$(CXX) $(CXXFLAGS) PNGimage/PNG.cpp

HSLAPixel.o : PNGimage/HSLAPixel.cpp PNGimage/HSLAPixel.h
	$(CXX) $(CXXFLAGS) PNGimage/HSLAPixel.cpp

lodepng.o : PNGimage/lodepng/lodepng.cpp PNGimage/lodepng/lodepng.h
	$(CXX) $(CXXFLAGS) PNGimage/lodepng/lodepng.cpp

clean :
	-rm -f *.o