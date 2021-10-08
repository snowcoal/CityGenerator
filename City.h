#pragma once

#include <iostream>
#include <vector>
#include <list>

#include "House.h"
#include "City.h"
#include "PNGimage/PNG.h"
#include "PNGimage/HSLAPixel.h"

using namespace PNGimage;
using namespace std;
using std::vector;
using std::string;
using std::list;
using std::cout;
using std::endl;

// percentages:
#define  PX_PCT                 0.5
#define  OUTER_RD_PCT           0.9
#define  HEIGHT_PCT             0.13
#define  LRBIAS                 90

// amounts of things:
#define  NUM_ADJ                8.0

// other:
#define  GRID_SPACE             1
#define  WHITE_PX_LUM           1
#define  BITMASK                0x03
#define  BITMASK2               0x01

// order array:
/*  lr  p | 4th | 3rd | 2nd | 1st | bin encode  | hex
* --------|-----|-----|-----|-----|-------------|------
*    0  0 |  0  |  1  |  2  |  3  | 11 10 01 00 | 0xe4
*    0  1 |  1  |  0  |  3  |  2  | 10 11 00 01 | 0xb1
*    1  0 |  2  |  3  |  0  |  1  | 01 00 11 10 | 0x4e
*    1  1 |  3  |  2  |  1  |  0  | 00 01 10 11 | 0x1b
*/
const int32_t ORDER_ARY[16] = {0, 1, 2, 3, 1, 0, 3, 2, 2, 3, 0, 1, 3, 2, 1, 0};

// basic pixel colors:
const HSLAPixel RED_PX(0.0,1.0,0.5);
const HSLAPixel GRN_PX(120,1.0,0.5);
const HSLAPixel BLU_PX(240,1.0,0.5);
const HSLAPixel OGE_PX(39,1.0,0.5);
const HSLAPixel BLK_PX(360,1.0,0.0);
const HSLAPixel BRN_PX(18, 0.58, 0.25);

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
            // x is left/right, z is up/down
            int32_t pos_x;
            int32_t pos_z;
            // vertical height - top left corner has y=0
            int32_t pos_y;
            // tracks the type of the cell
            // 0 = none, 1 = road, 2 = house, 3 = border
            int32_t type;
            // -1 = unassigned, -2 = house between corners, 0 = no neighbors, 1 = "i", 2 = "L", 3 = "T", 4 = "+"
            int32_t corner_type;
            // 0 = "i L T +", 1 = 90 clockwise, 2 = 180 clockwise, 3 = 270 clockwise, -1 unassigned
            // for non-corner houses: 0 rot = |----|, 1 rot = "I"
            // "+" gets assigned random rotation value
            int32_t corner_rotation;
            // tracks whether the current node is in the city or not
            bool inCity;
            // marks if visited for maze generator
            bool visited;
            // tracks if its a cliff or not
            bool isCliff;
            // average luminance of cell
            double avg_lum;
        };

        // // position of corner of city in the final map (needed later?)
        // int32_t start_x;
        // int32_t start_z;

        // number of cells in city
        int32_t cell_cnt;

        // input image pointer
        PNG* input_img;

        // heightmap input pointer
        PNG* heightmap_img;

        // number of steps on city from heightmap
        double num_steps;

        // length and width of the city
        int32_t cityWidth;
        int32_t cityLength;

        // length and width of grid
        int32_t gridWidth;
        int32_t gridLength;

        // width of each square (includes 1 block padding on each side)
        // 13 is default
        int32_t grid_cell_size;

        // bias of left/right vs up/down
        int32_t lrbias;

        // grid of city. Made up of grid nodes (for now might use houses later)
        // grid is made up of nxn squares with a border of 1 square around the entire image
        vector<vector<gridCell*>> grid;

        // list of all gridcell pointers in the city
        list<gridCell*> cityCells;

        // list of houses
        list<House*> houseList;

        // initializes stuff for constructors
        void init();

        // sets the maze height to the input heightmap (angles over some limit become dropoffs)
        void setCityHeight();

        // generates the road "maze" of the city
        int32_t generateRoads();

        // generates random True/False with given distribution
        bool randTF(int32_t dist);

        // picks a random neighbor
        gridCell* pickRandNeighbor(gridCell* cell, int32_t dist);

        // // house stuff

        // // loads house info into houseList
        // void loadHouses();

    public:
        // constructor for heightmap
        City(PNG* input, int32_t grid_box_width, int32_t lr_bias, PNG* heightmap, int32_t steps);

        // constructor for no heightmap
        City(PNG* input_img, int32_t grid_box_width, int32_t lr_bias);

        // prints out the city grid to input file
        void printGrid(string const & filename);

        // adds random roads along the maze
        void addRandomRoads(int32_t dist);

        // places houses into the city
        // they need to be randomized
        // direction of "stick" needs to be tracked
        void PlaceHouses();

        // // distorts the city (different architecture needed?)
        // void distortCity(int32_t dist);

        // Destructor
        ~City();

};