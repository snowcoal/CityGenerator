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

    City city(input, 11);

    // output.writeToFile("output.png");
    return 0;
}