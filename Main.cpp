// 2021 snowcoal
// I should probably add a CUI...

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

    // PNG cursed;
    // cursed.readFromFile("tests/input_cursed.png");

    PNG large;
    large.readFromFile("tests/input_large.png");

    PNG heightmap;
    heightmap.readFromFile("tests/input_heightmap.png");

    HouseSet houses("tests/input_new.csv");
    // houses.printTree();

    // for(int i = 0; i < 30; i++){
    //     int32_t h = houses.pickRandHouseByWidth(-2, 13, 13);
    //     cout << h << endl;
    // }

    // init seed for rand
    srand(69420);

    // // no heightmap input
    // City city0(&input, 13, 70, 100, 100);
    // city0.addRandomRoads(60);
    // city0.placeHouses(&houses);
    // city0.printGrid("tests/output.png");
    // city0.outputCityNoTP();

    // // no heightmap input
    // City city1(&input, 13, 10, 100, 100);
    // city1.placeHouses(&houses);
    // city1.printGrid("tests/output_maze.png");

    // // no heightmap input
    // City city2(&input, 13, 70, 100, 100);
    // city2.addRandomRoads(10);
    // city2.placeHouses(&houses);
    // city2.printGrid("tests/output2.png");
    
    // yes heightmap input
    City hcity(&input, 13, 70, 100, 100, &heightmap, 10);
    hcity.addRandomRoads(60);
    hcity.placeHouses(&houses);
    hcity.printGrid("tests/output_heightmap.png");
    hcity.outputCityNoTP();

    // City city3(&input2, 13, 70, 100, 100);
    // city3.addRandomRoads(60);
    // city3.placeHouses(&houses);
    // city3.printGrid("tests/output_small.png");
    // city3.outputCityNoTP();

    // // no heightmap input
    // City cityCursed(&cursed, 13, 70, 100, 100);
    // cityCursed.addRandomRoads(60);
    // cityCursed.placeHouses(&houses);
    // cityCursed.printGrid("tests/outputCursed.png");
    // cityCursed.outputCity();

    // // no heightmap input
    // City citylg(&large, 13, 70, 100, 100);
    // citylg.addRandomRoads(60);
    // citylg.placeHouses(&houses);
    // citylg.printGrid("tests/outputLarge.png");
    // citylg.outputCityNoTP();



    return 0;
}