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
    input.readFromFile("tests/input.png");

    PNG heightmap;
    heightmap.readFromFile("tests/input_heightmap.png");

    // no heightmap input
    City city(&input, 11, 100);
    city.addRandomRoads(50);
    city.printGrid("tests/output.png");

    // yes heightmap input
    City hcity(&input, 11, 20, &heightmap, 5);
    hcity.printGrid("tests/output_heightmap.png");

    return 0;
}