#pragma once

#include <iostream>
#include <vector>
#include <list>

#include "House.h"
#include "City.h"
#include "PNGimage/PNG.h"
#include "PNGimage/HSLAPixel.h"

#define  PX_PERCENTAGE          0.5
#define  GRID_SPACE             1
#define  WHITE_PX_LUM           1
#define  BLACK_PX_LUM           0
#define  OUTER_RD_PCT           0.8

using namespace PNGimage;
using namespace std;
using std::vector;
using std::string;
using std::list;
using std::cout;
using std::endl;

class City
{
    private:
        // node of city grid
        struct gridCell
        {
            // positions of node (0-based from top left corner of input image)
            // <---x--->
            /*
            * |
            * |
            * z
            * |
            * |
            */ 
            int32_t pos_x;
            int32_t pos_z;
            // vertical height - top left corner has y=0
            int32_t pos_y;
            // tracks whether the current node is in the city or not
            int32_t inCity;
            // tracks whether the current node is a road or not
            int32_t isRoad;
            // // tracks whether the current node is a space or not
            // int32_t isSpace;
        };

        // // position of corner of city in the final map (might not be needed)
        // int32_t start_x;
        // int32_t start_z;

        // input image pointer
        PNG* input_img;

        // length and width of the city
        int32_t cityWidth;
        int32_t cityLength;

        // length and width of grid
        int32_t gridWidth;
        int32_t gridLength;

        // width of each square (includes 1 block padding on each side)
        // 13 is default
        int32_t grid_cell_size;

        // grid of city. Made up of grid nodes (for now might use houses later)
        // grid is made up of nxn squares with a border of 1 square around the entire image
        vector<vector<gridCell*>> grid;

        // list of houses
        list<House*> houseList;

        // prints out the grid
        void PrintGrid();

        // generates the maze of the city
        void GenerateMaze();

        // // sets the maze height to the input heightmap (angles over some limit become dropoffs)
        // void SetCityHeight();

        // // distorts the city (different architecture needed?)
        // void DistortCity();

        // // house stuff

        // // loads houses into houseList
        // void loadHouses();

        // // places houses into the city
        // // they need to be randomized
        // // direction of "stick" needs to be tracked
        // void PlaceHouses();

    public:
        // Constructor - input will need to be heightmap and heightmap with marked areas for city
        // City nodes will need to be found and placed into the grid
        City(PNG input_img, int32_t grid_box_width);

        // // Copy constructor (probably not needed)
        // City();

        // Destructor
        ~City();

};