#include <iostream>
#include <vector>
#include <stdlib.h>

#include "City.h"
#include "HouseSet.h"
#include "PNGimage/PNG.h"
#include "PNGimage/HSLAPixel.h"

using namespace PNGimage;
using namespace std;

int main(){
    PNG input;
    input.readFromFile("tests/input.png");

    PNG input2;
    input2.readFromFile("tests/input_small.png");

    PNG heightmap;
    heightmap.readFromFile("tests/input_heightmap.png");

    HouseSet houses("tests/input.csv");
    // houses.printTree();

    // for(int i = 0; i < 30; i++){
    //     int32_t h = houses.pickRandHouseByWidth(-2, 13, 13);
    //     cout << h << endl;
    // }

    // init seed for rand
    srand(69420);

    // no heightmap input
    City city0(&input, 13, 70, 100, 100);
    city0.addRandomRoads(60);
    city0.placeHouses(&houses);
    city0.printGrid("tests/output.png");
    // city0.outputCity();

    // no heightmap input
    City city1(&input, 13, 10, 100, 100);
    city1.placeHouses(&houses);
    city1.printGrid("tests/output_maze.png");

    // no heightmap input
    City city2(&input, 13, 70, 100, 100);
    city2.addRandomRoads(10);
    city2.placeHouses(&houses);
    city2.printGrid("tests/output2.png");
    
    // yes heightmap input
    City hcity(&input, 13, 20, 100, 100, &heightmap, 2);
    hcity.addRandomRoads(40);
    hcity.placeHouses(&houses);
    hcity.printGrid("tests/output_heightmap.png");

    City city3(&input2, 13, 70, 100, 100);
    city3.addRandomRoads(60);
    city3.placeHouses(&houses);
    city3.printGrid("tests/output_small.png");
    city3.outputCity();



    return 0;
}