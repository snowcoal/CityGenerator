#include <iostream>
#include <vector>
#include <stdlib.h>

#include "City.h"
#include "House.h"
#include "PNGimage/PNG.h"
#include "PNGimage/HSLAPixel.h"

using namespace PNGimage;
using namespace std;

int main(){
    PNG input;
    input.readFromFile("input.png");

    // PNG heightmap;
    // heightmap.readFromFile("heightmap.png");

    // City city(&input, &heightmap, 11);
    City city(&input, 11);

    city.printGrid();
    return 0;
}