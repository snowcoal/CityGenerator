#pragma once

#include <iostream>
#include <vector>
#include <list>

#include "House.h"
#include "City.h"
#include "PNGimage/PNG.h"
#include "PNGimage/HSLAPixel.h"

#define  PX_PCT                 0.5
#define  GRID_SPACE             1
#define  WHITE_PX_LUM           1
#define  BLACK_PX_LUM           0
#define  OUTER_RD_PCT           0.9
#define  NUM_ADJ                8.0
#define  SEED                   69420
#define  HEIGHT_PCT             0.3
#define  NUM_BUCKETS            5.0

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
            // positions of node in grid
            int32_t i_index;
            int32_t j_index;
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
            // tracks the type of the cell
            // 0 = none, 1 = road, 2 = house, 3 = border, 4 = cliff
            int32_t type;
            // tracks whether the current node is in the city or not
            bool inCity;
            // marks if visited for maze generator
            bool visited;
        };

        // // position of corner of city in the final map (might not be needed)
        // int32_t start_x;
        // int32_t start_z;

        // number of cells in city
        int32_t cell_cnt;

        // input image pointer
        PNG* input_img;

        // heightmap input pointer
        PNG* heightmap_img;

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

        // list of all gridcell pointers in the city
        list<gridCell*> cityCells;

        // list of houses
        list<House*> houseList;

        // initializes stuff for constructors
        void init(PNG* input, int32_t grid_box_width);

        // sets the maze height to the input heightmap (angles over some limit become dropoffs)
        void setCityHeight();

        // generates the road "maze" of the city
        void generateRoads();

        // generates random True/False with given distribution
        bool randTF(int32_t dist);

        // gets 4 adjacent neighbors
        list<gridCell*>* get4Neighbors(gridCell* cell);

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
        // constructor for heightmap
        City(PNG* input_img, PNG* heightmap, int32_t grid_box_width);

        // constructor for no heightmap
        City(PNG* input_img, int32_t grid_box_width);

        // prints out the grid
        void printGrid();

        // Destructor
        ~City();

};