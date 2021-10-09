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

    // init seed for rand
    srand(69420);

    // no heightmap input
    City city0(&input, 11, 70);
    city0.addRandomRoads(50);
    city0.placeHouses();
    city0.printGrid("tests/output.png");

    // no heightmap input
    City city1(&input, 11, 70);
    city1.placeHouses();
    city1.printGrid("tests/output_maze.png");

    // no heightmap input
    City city2(&input, 11, 70);
    city2.addRandomRoads(10);
    city2.placeHouses();
    city2.printGrid("tests/output2.png");
    
    // yes heightmap input
    City hcity(&input, 11, 20, &heightmap, 5);
    hcity.printGrid("tests/output_heightmap.png");



    return 0;
}