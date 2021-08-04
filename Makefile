EXENAME = CityGenerator.exe
OBJS = main.o City.o House.o PNG.o HSLAPixel.o lodepng.o
#OUTPUTMSG = Maked files i guess

CXX = clang++
CXXFLAGS = $(CS225) -std=c++1y -stdlib=libc++ -c -g -O0 -Wall -Wextra -pedantic
LD = clang++
LDFLAGS = -std=c++1y -stdlib=libc++ -lc++abi -lm

.PHONY: all test clean output_msg

all : $(EXENAME)

#output_msg: $(OUTPUTMSG)

$(EXENAME) : output_msg $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) -o $(EXENAME)

# main:
	
main.o : main.cpp City.h House.h PNGimage/PNG.h PNGimage/HSLAPixel.h
	$(CXX) $(CXXFLAGS) main.cpp
 
# CityGenerator:

City.o: City.cpp City.h House.h PNGimage/PNG.h 
	$(CXX) $(CXXFLAGS) City.cpp
 
House.o: House.cpp House.h
	$(CXX) $(CXXFLAGS) House.cpp

# PNGimage:

PNG.o : PNGimage/PNG.cpp PNGimage/PNG.h PNGimage/HSLAPixel.h PNGimage/lodepng/lodepng.h
	$(CXX) $(CXXFLAGS) PNGimage/PNG.cpp

HSLAPixel.o : PNGimage/HSLAPixel.cpp PNGimage/HSLAPixel.h
	$(CXX) $(CXXFLAGS) PNGimage/HSLAPixel.cpp

lodepng.o : PNGimage/lodepng/lodepng.cpp PNGimage/lodepng/lodepng.h
	$(CXX) $(CXXFLAGS) PNGimage/lodepng/lodepng.cpp

.PHONY: output_msg

clean :
	-rm -f *.o $(EXENAME) test