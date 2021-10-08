#include <iostream>
#include <cmath>
#include <cstdlib>
#include <stack>
#include <list>

#include "City.h"
#include "PNGimage/PNG.h"
#include "PNGimage/HSLAPixel.h"

using namespace PNGimage;
using namespace std;

/*
* Constructor for no heightmap
*
* input_img - input PNG into constructor
* grid_box_width - width of grid boxes
* lr_bias - amount of bias to left/right vs up/down
*/
City::City(PNG* input, int32_t grid_box_width, int32_t lr_bias)
{
    // init basic private variables
    num_steps = 0;
    heightmap_img = NULL;
    input_img = input;
    grid_cell_size = grid_box_width;
    lrbias = lr_bias;
    init();
}

/*
* Constructor for heightmap
*
* input_img - input PNG into constructor
* heightmap - heightmap input
* grid_box_width - width of grid boxes
* steps - number of discrete height levels that the output should have
* lr_bias - amount of bias to left/right vs up/down
*/
City::City(PNG* input, int32_t grid_box_width, int32_t lr_bias, PNG* heightmap, int32_t steps)
{
    // init basic private variables
    num_steps = (double)steps;
    heightmap_img = heightmap;
    input_img = input;
    grid_cell_size = grid_box_width;
    lrbias = lr_bias;
    init();
}

/*
* Initialization method for both constructors
* 
*/
void City::init()
{
    // init basic private variables
    cityWidth = input_img->width();
    cityLength = input_img->height();
    cell_cnt = 0;
    // add outer edge of grid cells around original image
    gridWidth = cityWidth/(grid_cell_size + GRID_SPACE) + 4;
    gridLength = cityLength/(grid_cell_size + GRID_SPACE) + 4;
    // get area of cell and space
    int grid_cell_area = pow(grid_cell_size,2);

    // init grid cells
    for(int i = 0; i < gridLength; i++){
        // create empty row vector
        vector<gridCell*> row(gridWidth);
        for(int j = 0; j < gridWidth; j++){
            gridCell* cell = new gridCell;
            row[j] = cell;

            // the corner position needs to be -squarew,-squarew
            cell->pos_x = j * (grid_cell_size + GRID_SPACE) - (grid_cell_size);
            cell->pos_z = i * (grid_cell_size + GRID_SPACE) - (grid_cell_size);
            cell->pos_y = 0;
            cell->i_index = i;
            cell->j_index = j;
            cell->visited = false;
            cell->isCliff = false;
            cell->avg_lum = 0;
            cell->corner_type = -1;
            cell->corner_rotation = -1;
            // cells default to none type
            cell->type = 0;

            // find if the cell needs to be in city or not
            int count = 0;
            // loop for each pixel within the current grid cell
            for(int k = cell->pos_x; k < cell->pos_x + grid_cell_size; k++){
                for(int l = cell->pos_z; l < cell->pos_z + grid_cell_size; l++){
                    // check if checked pixel is in bounds
                    if(k>=0 && l>=0 && k<cityWidth && l<cityLength){
                        // get the current pixel
                        HSLAPixel & pixel = input_img->getPixel(k, l);
                        // check whether the luminance value is equal to the white pixel
                        if((int32_t)pixel.l == WHITE_PX_LUM){
                            count++;
                        }
                    }
                }
            }

            // check whether enough pixels are white
            if((double)count / (double)grid_cell_area > PX_PCT){
                cell->inCity = true;
                cell_cnt++;
                // add cell to list
                cityCells.push_back(cell);
            }
            else{
                cell->inCity = false;
            }
        }
        // add row to grid
        grid.push_back(row);
    }

    // set height of city if needed
    if(heightmap_img != NULL){
        setCityHeight();
    }

    // set the type of each cell prior to maze generation
    // uses "shitty edge detection"
    for(int i = 2; i < gridLength - 2; i++){
        for(int j = 2; j < gridWidth - 2; j++){
            gridCell* cell = grid[i][j];
            if(cell->inCity){
                // count 8 surrounding cells 
                int edge_cnt = 0;
                int height_cnt = 0;
                for(int k = -1; k <= 1; k++){
                    for(int l = -1; l <= 1; l++){
                        // count city cells in surrounding cells
                        if(grid[i+k][j+l]->inCity){
                            edge_cnt++;
                        }
                        // count lower height surrounding cells
                        if(grid[i+k][j+l]->pos_y < cell->pos_y){
                            height_cnt++;
                        }
                    }
                }
                // set it as a border if less than some percentage of its surrounding cells are in the city
                if((double)(edge_cnt-1) / NUM_ADJ < OUTER_RD_PCT){
                    cell->type = 3;
                    cell->visited = true;
                }
                // set it as a house if it has an odd position
                else if(!(i%2 == 0 && j%2 == 0)){
                    cell->type = 2;
                }
                // finally set it as a road
                else{
                    cell->type = 1;
                }
                // set it as a cliff if enough surrounding cells are different height and its not a border
                if((double)(height_cnt) / NUM_ADJ > HEIGHT_PCT && cell->type != 3){
                    cell->isCliff = true;
                }
            }
        }
    }

    // generate maze in city
    generateRoads();

}

/*
* setCityHeight
*
* sets the height of the city to input heightmap
*
*/
void City::setCityHeight()
{
    double grid_cell_area = (double)pow(grid_cell_size,2);
    double max_avg = 0.0;
    double min_avg = 1.0;
    // double avg_sum = 0;
    // loop through city cells
    for(auto cell: cityCells){
        double lum_sum = 0.0;
        // loop through pixels in cell
        for(int k = cell->pos_x; k < cell->pos_x + grid_cell_size; k++){
            for(int l = cell->pos_z; l < cell->pos_z + grid_cell_size; l++){
                // check if checked pixel is in bounds
                if(k>=0 && l>=0 && k<cityWidth && l<cityLength){
                    // get the current pixel
                    HSLAPixel & pixel = heightmap_img->getPixel(k, l);
                    // sum luminance
                    lum_sum += pixel.l;
                }       
            }
        }
        // calculate average
        double avg = lum_sum / grid_cell_area;
        // set max and min averages
        max_avg = max(max_avg, avg);
        min_avg = min(min_avg, avg);
        // store average
        cell->avg_lum = avg;
    }

    double range = max_avg - min_avg;
    // loop cells again and set yheights
    for(auto cell: cityCells){
        // scale average by range and truncate
        cell->pos_y = (int)floor((num_steps*(cell->avg_lum - min_avg))/range);
        // assertion check
        assert(cell->pos_y >= 0 && cell->pos_y <= num_steps);
    }
}

/*
* generateRoads
*
* generates maze of roads inside city where cliffs dont exsist
* returns 0 on success, -1 on failure
*
*/
int32_t City::generateRoads()
{
    // choose initial cell
    gridCell* initial = NULL;
    for(auto cell: cityCells){
        // find first road cell and break
        if(cell->type == 1){
            initial = cell;
            break;
        }
    }
    // if no initial cell is found, return -1
    if(initial == NULL){
        return -1;
    }

    // mark inital as visited and push to stack
    stack<gridCell*> frontier;
    frontier.push(initial);
    initial->visited = true;

    while(frontier.size() != 0){
        // current cell is top of stack
        gridCell* cur = frontier.top();
        frontier.pop();

        // if current has a neighbor
        gridCell* n = pickRandNeighbor(cur, lrbias);
        if(n != NULL){
            // push current to stack
            frontier.push(cur);

            // set wall between them to a road
            int32_t i_diff = (n->i_index - cur->i_index) >> 1;
            int32_t j_diff = (n->j_index - cur->j_index) >> 1;
            grid[cur->i_index + i_diff][cur->j_index + j_diff]->type = 1;
            //grid[cur->i_index + i_diff][cur->j_index + j_diff]->visited = true;

            // mark neighbor as visited and push it to stack
            n->visited = true;
            frontier.push(n);
        }
    }
    // return success
    return 0;
}

/*
* randTF
*
* generates a true/false random output with given dist
* dist is % chance it will be true
* only does integer percentages
*
*/
bool City::randTF(int32_t dist)
{
    // random num between 0 and 1000
    int x = rand() % 100;
    // true if the rand is <= to the dist
    return(x <= dist);
}

/*
* pickRandNeighbor
*
* given input cell, picks random neighboring adjacent cell
* that has not been visited. Used by maze generator.
* neighboring cell HAS to be 2 spaces away
*
* input lrbias is left/right bias as opposed to up/down
* 
* returns null if cell has no unvisited neighbors
*/
City::gridCell* City::pickRandNeighbor(gridCell* cell, int32_t lrbias)
{
    vector<gridCell*> cells(4);
    // uint32_t order;
    int32_t row;
    gridCell* output = NULL;
    int32_t i = cell->i_index;
    int32_t j = cell->j_index;
    // right/left
    cells[0] = grid[i][j+2];
    cells[1] = grid[i][j-2];
    // down/up
    cells[2] = grid[i+2][j];
    cells[3] = grid[i-2][j];

    // if true then left/right comes first
    bool lr = randTF(lrbias);
    // if true then plus comes first
    bool plus = randTF(50);

    // find which row of the order table to use
    if(!lr && !plus) row = 0;
    else if(!lr && plus) row = 1;
    else if(lr && !plus) row = 2;
    else row = 3;

    for(int n = 0; n < 4; n++){
        int index = ORDER_ARY[row*4 + n];
        // do only if cell is a road and is not visited
        if(!(cells[index]->visited) && cells[index]->type == 1){
            // update output
            output = cells[index];
            //std::cout<<index<<std::endl;
        }
    }

    // assertion check
    if(output != NULL){
        assert(output->type == 1 && !(output->visited));
    }

    return(output);
}

/*
* addRandomRoads
*
* adds random vertical/horizontal roads along the maze according to lrbias
* makes house loops
* connects extraneous houses together
*
* dist controls how many new roads get made
*
*/
void City::addRandomRoads(int32_t dist)
{

    // add vertical roads if most of the roads are horizontal so far
    if(lrbias > 50){
        for(int j = 2; j < gridWidth - 2; j++){
            // possibly skip entire column
            // the closer lrbias is to 50, the more columns are skipped
            if(!randTF((lrbias - 50)*2) || j%2 == 1){
                continue;
            }
            for(int i = 2; i < gridLength - 2; i++){
                gridCell* cell = grid[i][j];
                if(cell->inCity && cell->type == 2 && randTF(dist)){
                    cell->type = 1;
                }
            }
        }
    }

    // add horizontal roads if most of the roads are vertical so far
    else if(lrbias <= 50){
        for(int i = 2; i < gridLength - 2; i++){
            // possibly skip entire row
            // the closer lrbias is to 50, the more rows are skipped
            if(!randTF((50 - lrbias)*2) || i%2 == 1){
                continue;
            }
            for(int j = 2; j < gridWidth - 2; j++){
                gridCell* cell = grid[i][j];
                if(cell->inCity && cell->type == 2 && randTF(dist)){
                    cell->type = 1;
                }
            }
        }
    }

    // add house loops
    for(auto cell: cityCells){
        int32_t i = cell->i_index;
        int32_t j = cell->j_index;
        int house_cnt = 0;

        // loop outside cells of all road cells
        if(cell->type == 1){
            for(int k = -1; k <= 1; k++){
                for(int l = -1; l <= 1; l++){
                    if(grid[i+k][j+l]->type == 2 && !(k == 0 && l == 0)){
                        house_cnt++;
                    }
                }
            }
        }
        // check if theres exactly 7 houses and set any roads to houses
        if(house_cnt == 7){
            for(int k = -1; k <= 1; k++){
                for(int l = -1; l <= 1; l++){
                    if(grid[i+k][j+l]->type == 1 && !(k == 0 && l == 0)){
                        grid[i+k][j+l]->type = 2;
                    }
                }
            }
        }
    }

    // connect adjacent extraneous houses together (brute force algorithm)
    for(auto cell: cityCells){
        // skip any non-house cells
        if(cell->type != 2) continue;
        int32_t i = cell->i_index;
        int32_t j = cell->j_index;

        bool break1 = false;
        for(int k = -1; k <= 1; k++){
            if(break1) break;
            for(int l = -1; l <= 1; l++){
                if(k == 0 && l == 0) continue;
                // if any of them are not roads break all loops
                if(grid[i+k][j+l]->type != 1){
                    break1 = true;
                }
            }
        }
        // continue outer loop
        if(break1) continue;

        // if the cell is surrounded by roads, then check neighbors

        vector<gridCell*> cells(4);
        // right/left
        cells[0] = grid[i][j+2];
        cells[1] = grid[i][j-2];
        // down/up
        cells[2] = grid[i+2][j];
        cells[3] = grid[i-2][j];

        for(int n = 0; n < 4; n++){
            // skip any non-house cells
            if(cells[n]->type != 2) continue;

            bool break2 = false;
            for(int k = -1; k <= 1; k++){
                if(break2) break;
                for(int l = -1; l <= 1; l++){
                    if(k == 0 && l == 0) continue;
                    // if any of them are not roads break all loops
                    if(grid[cells[n]->i_index+k][cells[n]->j_index+l]->type != 1){
                        break2 = true;
                    }
                }
            }
            // continue outer loop
            if(break2) continue;
            // set cell between them to a house
            else{
                int32_t i_diff = (cells[n]->i_index - i) >> 1;
                int32_t j_diff = (cells[n]->j_index - j) >> 1;
                grid[i + i_diff][j + j_diff]->type = 2;
            }
        }
    }
}


/*
* placeHouses
*
* loads and places houses into city
*
*/
void City::PlaceHouses()
{
    //STEP 1: SETUP

    std::list<gridCell*> cornerCells;
    // first sort out all corner gridcells
    for(auto cell:cityCells){
        // first check if its a house
        if(cell->type == 2){
            // for each of the 4 direct neighbors, check if house (could error if house right next to edge)
            uint8_t n = 0x00;
            // shift n then OR with left neighbor
            n = ((n << 1) | ((grid[cell->i_index][cell->j_index - 1])->type == 2));
            // do same thing with right neighbor
            n = ((n << 1) | ((grid[cell->i_index][cell->j_index + 1])->type == 2));
            // do same thing with top neighbor
            n = ((n << 1) | ((grid[cell->i_index - 1][cell->j_index])->type == 2));
            // do same thing with bottom neighbor
            n = ((n << 1) | ((grid[cell->i_index + 1][cell->j_index])->type == 2));

            // use giant fucking switch statemet to find rotation and type of every single house

            // left|right|top|bottom
            // 0 rot = "i, L, T, +", 1 rot = 90, 2 rot = 180, 3 rot = 270
            // for non-corner houses: 0 rot = |----|, 1 rot = "I"
            switch (n){
                // no neighbors
                case 0b00000000:
                    cell->corner_type = 0;
                    cell->corner_rotation = rand() % 3;
                    cornerCells.push_front(cell);
                    break;
                // 1 neighbor
                case 0b00000001:
                    cell->corner_type = 1;
                    cell->corner_rotation = 0;
                    cornerCells.push_front(cell);
                    break;
                case 0b00000010:
                    cell->corner_type = 1;
                    cell->corner_rotation = 2;
                    cornerCells.push_front(cell);
                    break;
                case 0b00000100:
                    cell->corner_type = 1;
                    cell->corner_rotation = 3;
                    cornerCells.push_front(cell);
                    break;
                case 0b00001000:
                    cell->corner_type = 1;
                    cell->corner_rotation = 1;
                    cornerCells.push_front(cell);
                    break;
                // 2 neighbors no corner
                case 0b00000011:
                    cell->corner_type = -2;
                    cell->corner_rotation = 1;
                    break;
                case 0b00001100:
                    cell->corner_type = -2;
                    cell->corner_rotation = 0;
                    break;
                // 2 neighbors yes corner
                case 0b00000101:
                    cell->corner_type = 2;
                    cell->corner_rotation = 1;
                    cornerCells.push_front(cell);
                    break;
                case 0b00000110:
                    cell->corner_type = 2;
                    cell->corner_rotation = 0;
                    cornerCells.push_front(cell);
                    break;
                case 0b00001001:
                    cell->corner_type = 2;
                    cell->corner_rotation = 2;
                    cornerCells.push_front(cell);
                    break;
                case 0b00001010:
                    cell->corner_type = 2;
                    cell->corner_rotation = 3;
                    cornerCells.push_front(cell);
                    break;
                // 3 neighbors
                case 0b00000111:
                    cell->corner_type = 3;
                    cell->corner_rotation = 3;
                    cornerCells.push_front(cell);
                    break;
                case 0b00001011:
                    cell->corner_type = 3;
                    cell->corner_rotation = 1;
                    cornerCells.push_front(cell);
                    break;
                case 0b00001101:
                    cell->corner_type = 3;
                    cell->corner_rotation = 0;
                    cornerCells.push_front(cell);
                    break;
                case 0b00001110:
                    cell->corner_type = 3;
                    cell->corner_rotation = 2;
                    cornerCells.push_front(cell);
                    break;
                // 4 neighbors
                case 0b00001111:
                    cell->corner_type = 4;
                    cell->corner_rotation = rand() % 3;
                    cornerCells.push_front(cell);
                    break;
            }
        }
    }

    // find all lines between corners

    // find start, end, and direction of each line


    //STEP 2: PLACEMENT

    // first load all houses from csv

    // assign random houses of same type to corners

    // assign random houses to lines

    // need to ensure the direction/rotation of each house is specified
}


// /*
// * distortCity
// *
// * distorts the city by given percentage
// *
// * dist controls how much it gets distorted
// *
// */
// void City::distortCity(int32_t dist)
// {

// }

/*
* PrintGrid
*
* prints out the grid of the city to an input string
*
*/
void City::printGrid(string const & filename)
{
    PNG input_cpy = (*input_img);

    // loop through cells
    for(auto cell: cityCells){
        // find color that cell should be
        HSLAPixel color;
        switch (cell->type){
            // out of bounds
            case 0:
                color = BLK_PX;
                break;
            // road
            case 1:
                // color = BLU_PX;
                color = BLK_PX;
                break;
            // house
            case 2:
                color = RED_PX;
                // switch (cell->corner_type){
                //     case -2:
                //         color = RED_PX;
                //         break;
                //     case 0:
                //         color = RED_PX;
                //         break;
                //     case 1:
                //         color = RED_PX;
                //         break;
                //     case 2:
                //         color = RED_PX;
                //         break;
                //     case 3:
                //         color = RED_PX;
                //         break;
                //     case 4:
                //         color = BLU_PX;
                //         break;
                // }

                // switch (cell->corner_rotation){
                //     case 0:
                //         color = RED_PX;
                //         break;
                //     case 1:
                //         color = RED_PX;
                //         break;
                //     case 2:
                //         color = RED_PX;
                //         break;
                //     case 3:
                //         color = BLU_PX;
                //         break;
                //     default:
                //         color = RED_PX;
                //         break;
                // }
                break;
            // border
            case 3:
                color = OGE_PX;
                break;
            // // corner
            // case 4:
            //     color = BLU_PX;
            //     break;
        }
        if(cell->isCliff) color = GRN_PX;
        // loop through each pixel in the cell
        for(int k = cell->pos_x; k < cell->pos_x + grid_cell_size; k++){
            for(int l = cell->pos_z; l < cell->pos_z + grid_cell_size; l++){
                // check if checked pixel is in bounds
                if(k>=0 && l>=0 && k<cityWidth && l<cityLength){
                    // set the pixel to the cell's color
                    HSLAPixel & pixel = input_cpy.getPixel(k, l);
                    pixel = color;
                }
            }
        }
    }
    input_cpy.writeToFile(filename);
}

/*
* Destructor
*
* frees memory
*
*/
City::~City(){
    for(int i = 0; i < gridLength; i++){
        for(int j = 0; j < gridWidth; j++){
            free(grid[i][j]);
        }
    }
}