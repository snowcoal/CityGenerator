// 2021 snowcoal
// Yeah i probably shouldve split this into more classes but whatever

#include <iostream>
#include <cmath>
#include <cstdlib>
#include <stack>
#include <queue>
#include <list>
#include <fstream>
#include <algorithm>

#include "City.h"
#include "HouseSet.h"
#include "PNGimage/PNG.h"
#include "PNGimage/HSLAPixel.h"

using namespace PNGimage;
using namespace std;

/*
* Constructor for no heightmap
*
* input - the input PNG that defines where the city is to go
* grid_box_width - width of grid boxes
* lr_bias - amount of bias to left/right vs up/down
* x_corner - the x position of the top left corner of the city
* z_corner - the z position of the top left corner of the city
*/
City::City(PNG* input, int32_t grid_box_width, int32_t lr_bias, int32_t x_corner, int32_t z_corner)
{   
    // init basic private variables
    num_steps = 0;
    heightmap_img = NULL;
    input_img = input;
    grid_cell_size = grid_box_width;
    lrbias = lr_bias;
    start_x = x_corner;
    start_z = z_corner;
    init();
}

/*
* Constructor for heightmap
*
* input - the input PNG that defines where the city is to go
* heightmap - heightmap input
* grid_box_width - width of grid boxes
* steps - number of discrete height levels that the output should have
* lr_bias - amount of bias to left/right vs up/down
* x_corner - the x position of the top left corner of the city
* z_corner - the z position of the top left corner of the city
*/
City::City(PNG* input, int32_t grid_box_width, int32_t lr_bias, int32_t x_corner, int32_t z_corner, PNG* heightmap, int32_t steps)
{
    // init basic private variables
    num_steps = (double)steps;
    heightmap_img = heightmap;
    input_img = input;
    grid_cell_size = grid_box_width;
    lrbias = lr_bias;
    start_x = x_corner;
    start_z = z_corner;
    init();
}

/*
* Initialization method for both constructors
* 
*/
void City::init()
{
    // check inputs
    if(grid_cell_size%2 == 0){
        cout<<"ERROR: grid_box_width input must be an odd number!"<<endl;
        return;
    }
    if(lrbias < 0 || lrbias > 100){
        cout<<"ERROR: lr_bias must be in the range of 0-100!"<<endl;
        return;
    }

    // init basic private variables
    cityWidth = input_img->width();
    cityLength = input_img->height();
    cell_cnt = 0;
    placeHousesCalled = false;
    // add outer edge of grid cells around original image
    gridWidth = cityWidth/(grid_cell_size + GRID_SPACE) + 4;
    gridLength = cityLength/(grid_cell_size + GRID_SPACE) + 4;
    // get area of cell and space
    int grid_cell_area = pow(grid_cell_size,2);

    half_grid_space = ((GRID_SPACE - 1) >> 2) + 1;

    // allocate various lists
    list<list<gridCell*>*>* l1 = new list<list<gridCell*>*>;
    lineList = l1;

    list<gridCell*>* l2 = new list<gridCell*>;
    cityCells = l2;

    list<cityHouse*>* l3 = new list<cityHouse*>;
    houseList = l3;

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
            cell->pos_y = DEFAULT_Y_HEIGHT;
            cell->i_index = i;
            cell->j_index = j;
            cell->visited = false;
            cell->isCliff = false;
            cell->visitedLine = false;
            cell->visitedBFS = false;
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
                cityCells->push_back(cell);
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
        setCityHeight(1);
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
                // if((double)(height_cnt) / NUM_ADJ > HEIGHT_PCT && cell->type != 3){
                //     cell->isCliff = true;
                //     // cell->type = 5;
                // }
            }
        }
    }

    // generate maze in city
    generateRoads();

    // output msg
    cout <<"City Generation Complete. "<< cityCells->size() <<" cells were generated."<<endl;
}

/*
* setCityHeight
*
* sets the height of the city to input heightmap
*
*/
void City::setCityHeight(int32_t stepSize)
{
    double grid_cell_area = (double)pow(grid_cell_size,2);
    double max_avg = 0.0;
    double min_avg = 1.0;
    // double avg_sum = 0;
    // loop through city cells
    for(auto cell: *cityCells){
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
    for(auto cell: *cityCells){
        // scale average by range and truncate
        cell->pos_y += (int)floor((num_steps*(cell->avg_lum - min_avg))/range) * stepSize;
        // assertion check
        // assert(cell->pos_y >= 0 && cell->pos_y <= num_steps);
    }
}

// smooth city to terrain

// cannot have a height differrence of >1 between each house

// steps:

// 1. start with random cell

// 2. get BFS of all houses in current height area (edges are cliffs and walls)

// 2a. if cell has less average luminance than previous, then decrease height
// 2b. else increase height



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
    for(auto cell: *cityCells){
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
    if(dist < 0 || dist > 100){
        cout<<"ERROR: dist must be in the range of 0-100!"<<endl;
        return;
    }

    if(placeHousesCalled){
        cout<<"ERROR: addRandomRoads() cannot be called after placeHouses()!"<<endl;
        return;
    }
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

    // list to hold new house cells
    list<gridCell*>* newHouses = new list<gridCell*>;
    // list to hold new road cells
    list<gridCell*>* roads = new list<gridCell*>;
    // list to hold new road cells
    list<gridCell*>* houses = new list<gridCell*>;

    // add house loops 2x
    for(int i = 0; i < NUM_LOOP_PASSES; i++){
        for(auto cell: *cityCells){
            int32_t house_cnt = 0;
            int32_t road_cnt = 0;

            // check if the cell is a road and hasnt been visted yet
            if(cell->type == 1 || cell->type == 2){
                cellCount cnt = getNumNeighbors(cell);
                house_cnt = cnt.houseCount;
                road_cnt = cnt.roadCount;
            }
            // otherwise skip it
            else continue;

            // only edit if it has exactly 7 houses around it
            if(house_cnt == 7){
                int32_t i = cell->i_index;
                int32_t j = cell->j_index;

                // add it to the newRoads list to check later
                roads->push_back(cell);

                // find extra road
                for(int k = -1; k <= 1; k++){
                    for(int l = -1; l <= 1; l++){
                        // the following should only happen one time per double loop
                        if(grid[i+k][j+l]->type == 1 && !(k == 0 && l == 0)){
                            // set the road to a house
                            gridCell* road = grid[i+k][j+l];
                            // road->type = 2;
                            newHouses->push_back(road);
                            // goto to break double loop idgaf
                            goto end_loop0;
                        }
                    }
                }
                end_loop0:
                continue;
            }
            // only add to the list once
            if(road_cnt == 8 && i == 0){
                houses->push_back(cell);
            }
        }

        for(auto road: *newHouses){
            road->type = 2;
        }
        newHouses->clear(); 
    }

    // add a few more house cells
    combineAdjacentCells(roads, 1, 2);
    delete roads;

    // add a few more road cells
    combineAdjacentCells(houses, 2, 1);
    delete houses;

    delete newHouses;

    // output msg
    cout <<"Random roads added to city. "<<endl;
}

/*
* getNumNeighbors
*
* looks through cellList for cells that have same type 2 cells apart and both surrounded
* by cells of a different type. It then removes the cell between these two
*
*
* cellList - list of cells to check over
* baseType - type to set the cells to if needed, and type of center cell
* checkType - type to check the surrounding cells for
*/
void City::combineAdjacentCells(list<gridCell*>* cellList, int32_t baseType, int32_t checkType)
{
    list<gridCell*> newCellList;
    // loop through all potential cells that could have a new connecting road
    for(auto cell: *cellList){
        int32_t i = cell->i_index;
        int32_t j = cell->j_index;
        // get 4 neighbors
        gridCell* neighbors[4] = {grid[i][j+2], grid[i][j-2], grid[i+2][j], grid[i-2][j]};
        for(int k = 0; k < 4; k++){
            gridCell* ncell = neighbors[k];
            // check if the neighbors is a road
            if(ncell->type == baseType){
                cellCount cnt = getNumNeighbors(ncell);
                int32_t x = 0;
                if(checkType == 1){
                    x = cnt.roadCount;
                }
                else if(checkType == 2){
                    x = cnt.houseCount;
                }
                if(x == 8){
                    int32_t i_diff = (ncell->i_index - i) >> 1;
                    int32_t j_diff = (ncell->j_index - j) >> 1;
                    // set cell between them to house
                    gridCell* new_cell = grid[i + i_diff][j + j_diff];
                    if(new_cell->type == checkType)
                    newCellList.push_back(new_cell);
                    // grid[i + i_diff][j + j_diff]->type = baseType;
                }
            }
        }
    }
    for(auto house: newCellList){
        house->type = baseType;
    }
}

/*
* getNumNeighbors
*
* gets the number of neighboring cells that have type input
*
*/
City::cellCount City::getNumNeighbors(gridCell* cell)
{   
    cellCount retVal;
    retVal.roadCount = 0;
    retVal.houseCount = 0;

    int32_t i = cell->i_index;
    int32_t j = cell->j_index;
    
    for(int k = -1; k <= 1; k++){
        for(int l = -1; l <= 1; l++){
            if(!(k == 0 && l == 0)){
                gridCell* ncell = grid[i+k][j+l];
                // update road counter
                if((ncell->type == 1 || ncell->isCliff) && ncell->type != 2) retVal.roadCount++;
                // update house counter
                else if((ncell->type == 2 || ncell->isCliff) && ncell->type != 1) retVal.houseCount++;
            }
        }
    }

    return retVal;
}


/*
* placeHouses
*
* loads and places houses into city
*
*/
void City::placeHouses(HouseSet* house_set)
{
    if(placeHousesCalled){
        cout<<"ERROR: placeHouses() cannot be called more than once!"<<endl;
        return;
    }

    // first ensure cliffs are correct
    for(auto cell: *cityCells){
        if(cell->isCliff){
            cell->type = 5;
        }
    }

    //STEP 1: SETUP

    placeHousesCalled = true;

    list<gridCell*> cornerCells;
    list<gridCell*> lineCells;
    // first sort out all corner gridcells
    for(auto cell: *cityCells){
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
            // for non-corner houses: 0 rot = H, 1 rot = "I"
            switch (n){
                // no neighbors
                case 0b00000000:
                    cell->corner_type = 0;
                    cell->corner_rotation = rand() % 3;
                    cornerCells.push_back(cell);
                    break;
                // 1 neighbor
                case 0b00000001:
                    cell->corner_type = 1;
                    cell->corner_rotation = 2;
                    cornerCells.push_back(cell);
                    break;
                case 0b00000010:
                    cell->corner_type = 1;
                    cell->corner_rotation = 0;
                    cornerCells.push_back(cell);
                    break;
                case 0b00000100:
                    cell->corner_type = 1;
                    cell->corner_rotation = 1;
                    cornerCells.push_back(cell);
                    break;
                case 0b00001000:
                    cell->corner_type = 1;
                    cell->corner_rotation = 3;
                    cornerCells.push_back(cell);
                    break;
                // 2 neighbors no corner
                case 0b00000011:
                    cell->corner_type = -2;
                    cell->corner_rotation = 1;
                    lineCells.push_back(cell);
                    break;
                case 0b00001100:
                    cell->corner_type = -2;
                    cell->corner_rotation = 0;
                    lineCells.push_back(cell);
                    break;
                // 2 neighbors yes corner
                case 0b00000101:
                    cell->corner_type = 2;
                    cell->corner_rotation = 1;
                    cornerCells.push_back(cell);
                    break;
                case 0b00000110:
                    cell->corner_type = 2;
                    cell->corner_rotation = 0;
                    cornerCells.push_back(cell);
                    break;
                case 0b00001001:
                    cell->corner_type = 2;
                    cell->corner_rotation = 2;
                    cornerCells.push_back(cell);
                    break;
                case 0b00001010:
                    cell->corner_type = 2;
                    cell->corner_rotation = 3;
                    cornerCells.push_back(cell);
                    break;
                // 3 neighbors
                case 0b00000111:
                    cell->corner_type = 3;
                    cell->corner_rotation = 3;
                    cornerCells.push_back(cell);
                    break;
                case 0b00001011:
                    cell->corner_type = 3;
                    cell->corner_rotation = 1;
                    cornerCells.push_back(cell);
                    break;
                case 0b00001101:
                    cell->corner_type = 3;
                    cell->corner_rotation = 0;
                    cornerCells.push_back(cell);
                    break;
                case 0b00001110:
                    cell->corner_type = 3;
                    cell->corner_rotation = 2;
                    cornerCells.push_back(cell);
                    break;
                // 4 neighbors
                case 0b00001111:
                    cell->corner_type = 4;
                    cell->corner_rotation = rand() % 3;
                    cornerCells.push_back(cell);
                    break;
            }
        }
    }

    // find all lines between corners
    for(auto cell:lineCells){
        // first check if its not been visited
        if(!cell->visitedLine){
            // mark it as visited
            cell->visitedLine = true;
            // allocate a new line
            list<gridCell*>* cellLine = new list<gridCell*>;
            // add the line to the line list
            lineList->push_back(cellLine);
            // add the cell to the line
            cellLine->push_back(cell);

            // if rotation is "H", search right
            if(cell->corner_rotation == 0){
                gridCell* rightCell = cell;
                gridCell* cur_cell = cell;
                while(1){
                    // set current to the cell to the right of the initial one
                    rightCell = grid[cur_cell->i_index][cur_cell->j_index + 1];
                    // if it hasent been visited and its a house between corners, then add it to list
                    if(rightCell->corner_type == -2){
                        cellLine->push_back(rightCell);
                        rightCell->visitedLine = true;
                        cur_cell = rightCell;
                    }
                    // end loop if it hits a cell that isnt a house between corners
                    else{
                        break;
                    }
                }
            }
            // if rotation is "I", search down
            else if(cell->corner_rotation == 1){
                gridCell* downCell = cell;
                gridCell* cur_cell = cell;
                while(1){
                    // set current to the cell under the initial one
                    downCell = grid[cur_cell->i_index + 1][cur_cell->j_index];
                    // if it hasent been visited and its a house between corners, then add it to list
                    if(downCell->corner_type == -2){
                        cellLine->push_back(downCell);
                        downCell->visitedLine = true;
                        cur_cell = downCell;
                    }
                    // end loop if it hits a cell that isnt a house between corners
                    else{
                        break;
                    }
                }
            }
        }
    }

    // // debugging function
    // for(auto line:*lineList){
    //     cout << line->size() << " " << line->front()->corner_rotation << endl;
    //     for(auto cell:*line){
    //         cell->type = 4;
    //     }
    // }

    //STEP 2: PLACEMENT

    // assign houses to all corners
    assignHousesToCorners(house_set, &cornerCells);

    // assign houses to all lines
    assignHousesToLines(house_set);

    // assign remaining splits
    for(auto cell: *cityCells){
        int32_t i = cell->i_index;
        int32_t j = cell->j_index;
        // check if cell has even position and if its a corner
        if((i%2 == 0 || j%2 == 0) && cell->corner_type >= 1){
            // check surrounding cells to find if any of them are corners
            gridCell* neighbors[4] = {grid[i][j+1], grid[i][j-1], grid[i+1][j], grid[i-1][j]};
            for(int n = 0; n < 4; n++){
                gridCell* ncell = neighbors[n];
                if(ncell->corner_type >= 1){
                    // calculate constants
                    int32_t z_diff = (ncell->i_index - i);
                    int32_t x_diff = (ncell->j_index - j);
                    int32_t offset = ((grid_cell_size - 1) >> 1) + 1;
                    int32_t posx = cell->pos_x + offset;
                    int32_t posz = cell->pos_z + offset;
                    int32_t rot = -1;
                    // calculate rotation
                    if(z_diff == 0) rot = 0;
                    else if(x_diff == 0) rot = 1;

                    // add a new split (idk why this works but it does via trial/error)
                    addSplit(posx + x_diff*(offset) - abs(x_diff), cell->pos_y, posz + z_diff*(offset) - abs(z_diff), rot, house_set);
                }
            }
        }
    }

    // output msg
    cout <<"House Placement Complete. "<< houseList->size() <<" houses were placed."<<endl;
}

/*
* assignHousesToCorners
*
* assigns houses to all corners. Inputs are the house set and corner cell set
*
*/
void City::assignHousesToCorners(HouseSet* house_set, list<gridCell*>* cornerCells)
{
    for(auto cell: *cornerCells){
        // get a random house with same type
        HouseSet::houseType* houseID = house_set->pickRandHouseByWidth(cell->corner_type, grid_cell_size, grid_cell_size);
        if(houseID == NULL){
            cout << "ERROR: could not find house for cell" << endl;
            return;
        }

        // make new city house and set its data
        cityHouse* city_house = new cityHouse;
        
        // location is at CENTER of house 
        city_house->pos_x = cell->pos_x + ((grid_cell_size - 1) >> 1) + 1;
        city_house->pos_y = cell->pos_y;
        city_house->pos_z = cell->pos_z + ((grid_cell_size - 1) >> 1) + 1;

        city_house->rotation = cell->corner_rotation;
        city_house->house_ptr = houseID;

        houseList->push_back(city_house);
    }
}

/*
* assignHousesToLines
*
* assigns houses to all corners Inputs are the house set (line list set is a member var)
*
*/
void City::assignHousesToLines(HouseSet* house_set)
{
    for(auto line:*lineList){
        // calculate various needed values
        int32_t line_cell_width = line->size();
        int32_t line_rotation = line->front()->corner_rotation;
        // int32_t line_px_width = GRID_SPACE + GRID_SPACE * line_cell_width + grid_cell_size * line_cell_width;
        // store beginning and end of line
        gridCell* line_begin = line->front();
        // store y val of line
        int32_t line_y_val = line_begin->pos_y;


        vector<cityHouse*> lineHouses;
        
        // keep track of how many houses have been placed
        int32_t count = 0;
        while(1){
            int32_t houses_left = line_cell_width - count;

            if(houses_left > 1){
                // pick a random house of width >= grid_cell_size
                HouseSet::houseType* houseID = house_set->pickRandHouseByWidth(-2, grid_cell_size + 1, INT32_MAX);
                // add the first house
                cityHouse* city_house = new cityHouse;
                // assign its rotation and ID pointer
                city_house->house_ptr = houseID;
                city_house->rotation = line_rotation;
                lineHouses.push_back(city_house);
                count++;

                int32_t width = getCityHouseWidth(city_house);
                // if the first house was a grid_cell_size house, dont add another
                if(width == grid_cell_size){
                    continue;
                }
                // if the first house was grid_cell_size * 2 + 1, update count and dont add another
                else if(width == grid_cell_size*2 + 1){
                    count++;
                    continue;
                }
                // otherwise add a smaller house of corresponding width to add 2 houses total during current pass
                else{
                    int32_t width2 = 2*grid_cell_size - width;
                    // pick a random house of corresponding width to add to 2 houses
                    HouseSet::houseType* houseID2 = house_set->pickRandHouseByWidth(-2, width2, width2);
                    // add the 2nd house
                    cityHouse* city_house2 = new cityHouse;
                    city_house2->house_ptr = houseID2;
                    city_house2->rotation = line_rotation;
                    lineHouses.push_back(city_house2);
                    count++;
                }
            }
            else if(houses_left == 1){
                // add a house of width equal to grid cell size
                HouseSet::houseType* houseID = house_set->pickRandHouseByWidth(-2, grid_cell_size, grid_cell_size);
                cityHouse* city_house = new cityHouse;
                // assign its rotation and ID pointer
                city_house->house_ptr = houseID;
                city_house->rotation = line_rotation;
                lineHouses.push_back(city_house);
                count++;
            }
            // break if theres no more houses to add
            else if(houses_left == 0){
                break;
            }
        }

        // // assertion check
        // int32_t total_width = 0;
        // for(auto house:lineHouses){
        //     total_width += getCityHouseWidth(house);
        // }
        // total_width += (lineHouses.size() + 1);
        // assert(total_width == line_px_width);


        // randomly shuffle vector
        std::random_shuffle(lineHouses.begin(), lineHouses.end());

        int32_t line_z_val = 0;
        int32_t line_x_val = 0;

        // do if line goes from left to right ("H")
        if(line_rotation == 0){
            // if line goes from left to right, its Z val is constant and is the centered line in the middle of the cells
            line_z_val = line_begin->pos_z + ((grid_cell_size - 1) >> 1) + 1;
            line_x_val = line_begin->pos_x - half_grid_space;

            for(auto house: lineHouses){
                // add a split
                addSplit(line_x_val, line_y_val, line_z_val, line_rotation, house_set);
                line_x_val += GRID_SPACE;
                // set the houses positional data and add it to the list
                int32_t width = getCityHouseWidth(house);
                house->pos_z = line_z_val;
                house->pos_y = line_y_val;
                house->pos_x = line_x_val + ((width - 1) >> 1) + 1;

                houseList->push_back(house);

                line_x_val += width;
            }
            // add last split
            addSplit(line_x_val, line_y_val, line_z_val, line_rotation, house_set);
        }
        // do if line goes from top to bottom ("I")
        else if(line_rotation == 1){
            // if line goes from top to bottom, its X val is constant and is the centered line in the middle of the cells
            line_x_val = line_begin->pos_x + ((grid_cell_size - 1) >> 1) + 1;
            line_z_val = line_begin->pos_z - half_grid_space;

            for(auto house: lineHouses){
                // add a split
                addSplit(line_x_val, line_y_val, line_z_val, line_rotation, house_set);
                line_z_val += GRID_SPACE;
                // set the houses positional data and add it to the list
                int32_t width = getCityHouseWidth(house);
                house->pos_z = line_z_val + ((width - 1) >> 1) + 1;
                house->pos_y = line_y_val;
                house->pos_x = line_x_val;

                houseList->push_back(house);

                line_z_val += width;
            }
            // add last split
            addSplit(line_x_val, line_y_val, line_z_val, line_rotation, house_set);
        }
    }
}

/*
* addSplit
*
* adds a new split type house to the house set
*
*
*/
void City::addSplit(int32_t posx, int32_t posy, int32_t posz, int32_t rot, HouseSet* house_set)
{
    // add a house of width 1
    HouseSet::houseType* houseID = house_set->pickRandHouseByWidth(-2, 1, 1);
    cityHouse* city_house = new cityHouse;

    if(!rot){
        city_house->pos_x = posx + ((GRID_SPACE - 1) >> 1) + 1;
        city_house->pos_z = posz;
    }
    else if(rot){
        city_house->pos_x = posx;
        city_house->pos_z = posz + ((GRID_SPACE - 1) >> 1) + 1;
    }

    // set its information
    city_house->pos_y = posy;

    city_house->rotation = rot;
    city_house->house_ptr = houseID;

    // add it to the list
    houseList->push_back(city_house);
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
//     if(!placeHousesCalled){
//         cout<<"ERROR: distortCity() can only be called after placeHouses()!"<<endl;
//         return;
//     }
// }

/*
* outputCity
*
* generates a set of worldedit commands to be run to generate the city. Requires player teleportation.
*
*/
void City::outputCity()
{
    if(!placeHousesCalled){
        cout<<"ERROR: outputCity() can only be called after placeHouses()!"<<endl;
        return;
    }

    vector<cityHouse> houseList2;

    // not memory efficient but i couldnt get it working a different way
    for(auto house: *houseList){
        houseList2.push_back(*house);
    }

    // sort list first by width then by ID. This ensures all IDs are next to each other and split houses come first
    sort(houseList2.begin(), houseList2.end(), [](const cityHouse& a, const cityHouse& b){
        int32_t a_ID = a.house_ptr->ID;
        int32_t b_ID = b.house_ptr->ID;
        int32_t a_w = a.house_ptr->width;
        int32_t b_w = b.house_ptr->width;
        int32_t a_r = a.rotation;
        int32_t b_r = b.rotation;
        if(a_w != b_w){
            return a_w < b_w;
        }
        // if widths are equal sort by ID
        else if(a_ID != b_ID){
            return a_ID < b_ID;
        }
        // otherwise sort by rotation
        else{
            return a_r < b_r;
        }
    });

    // open file
    fstream fp;
    fp.open("tests/output1.txt", ios::out);

    if(!fp) cout << "ERROR: could not write to output file" << endl;

    int32_t cur_ID = -69;
    int32_t cur_rot = -69;
    bool newCopyFlag = false;
    // loop through ordered list
    for(auto house: houseList2){
        HouseSet::houseType* housePtr = house.house_ptr;
        int32_t houseID = housePtr->ID;
        // int32_t houseType = getCityHouseType(&house);
        int32_t houseRot = house.rotation;
        // do this whenever a new houseID comes up
        if(houseID != cur_ID){
            cur_ID = houseID;
            // copy the next house in game
            // tp player to copy location
            fp <<"tp @s "<< housePtr->cpy_pt_x <<" "<< housePtr->cpy_pt_y <<" "<< housePtr->cpy_pt_z << endl;
            // set pos1
            fp <<"/pos1 "<< housePtr->pos1_x <<","<< housePtr->pos1_y <<","<< housePtr->pos1_z << endl;
            // set pos2
            fp <<"/pos2 "<< housePtr->pos2_x <<","<< housePtr->pos2_y <<","<< housePtr->pos2_z << endl;
            // do copy with no bedrock
            fp <<"/copy -m !bedrock"<< endl;

            // set flag that new house was copied
            newCopyFlag = true;

            // initialize current rotation to 0
            cur_rot = 0;
        }

        // tp player to the paste location
        fp <<"tp @s "<< house.pos_x + start_x <<" "<< house.pos_y <<" "<< house.pos_z + start_z << endl;

        // do this whenever a new rotation comes up
        if(houseRot != cur_rot){
            // dont neg rotate it the first time or if a house was just copied
            if(cur_rot != 0 && !newCopyFlag){
                // rotate it by the old rotation in the negative direction
                fp <<"/rotate "<< cur_rot * -90 << endl;
            }
            // rotate the house if needed
            if(houseRot != 0){
                fp <<"/rotate "<< houseRot * 90 << endl;
            }
            cur_rot = houseRot;
        }
        // paste the house in no air blocks
        fp <<"/paste -a"<< endl;

        // reset new copy flag
        newCopyFlag = false;
    }

    fp.close();
    // output msg
    cout <<"City output as output.txt."<<endl;
}


/*
* outputCity
*
* outputs the city in such a way that teleporting is not needed. Compatible with worldedit CLI
* if extra slashes are added
*
*/
void City::outputCityNoTP()
{
    if(!placeHousesCalled){
        cout<<"ERROR: outputCity() can only be called after placeHouses()!"<<endl;
        return;
    }

    vector<cityHouse> houseList2;

    // not memory efficient but i couldnt get it working a different way
    for(auto house: *houseList){
        houseList2.push_back(*house);
    }

    // sort list first by width then by ID. This ensures all IDs are next to each other and split houses come first
    sort(houseList2.begin(), houseList2.end(), [](const cityHouse& a, const cityHouse& b){
        int32_t a_ID = a.house_ptr->ID;
        int32_t b_ID = b.house_ptr->ID;
        int32_t a_w = a.house_ptr->width;
        int32_t b_w = b.house_ptr->width;
        int32_t a_r = a.rotation;
        int32_t b_r = b.rotation;
        if(a_w != b_w){
            return a_w < b_w;
        }
        // if widths are equal sort by ID
        else if(a_ID != b_ID){
            return a_ID < b_ID;
        }
        // otherwise sort by rotation
        else{
            return a_r < b_r;
        }
    });

    // open file
    fstream fp;
    fp.open("tests/output.txt", ios::out);

    if(!fp) cout << "ERROR: could not write to output file" << endl;

    fp << "toggleplace" << endl;

    int32_t cur_ID = -69;
    int32_t cur_rot = -69;
    bool newCopyFlag = false;
    // loop through ordered list
    for(auto house: houseList2){
        HouseSet::houseType* housePtr = house.house_ptr;
        int32_t houseID = housePtr->ID;
        // get some tmp variables for later
        int32_t py = house.pos_y;
        int32_t dx = abs(housePtr->cpy_pt_x - housePtr->pos1_x);
        int32_t dz = abs(housePtr->cpy_pt_z - housePtr->pos1_z);
        // int32_t houseType = getCityHouseType(&house);
        int32_t houseRot = house.rotation;

        // do this whenever a new houseID comes up
        if(houseID != cur_ID){
            cur_ID = houseID;
            // copy the next house in game

            // set pos1
            fp <<"/pos1 "<< housePtr->pos1_x <<","<< housePtr->pos1_y <<","<< housePtr->pos1_z << endl;
            // set pos2
            fp <<"/pos2 "<< housePtr->pos2_x <<","<< housePtr->pos2_y <<","<< housePtr->pos2_z << endl;
            // do copy with no bedrock
            fp <<"/copy -m !bedrock"<< endl;

            // set flag that new house was copied
            newCopyFlag = true;

            // initialize current rotation to 0
            cur_rot = 0;
        }

        // do this whenever a new rotation comes up
        if(houseRot != cur_rot){
            // dont neg rotate it the first time or if a house was just copied
            if(cur_rot != 0 && !newCopyFlag){
                // rotate it by the old rotation in the negative direction
                fp <<"/rotate "<< cur_rot * -90 << endl;
            }
            // rotate the house if needed
            if(houseRot != 0){
                fp <<"/rotate "<< houseRot * 90 << endl;
            }
            cur_rot = houseRot;
        }
        // calculate paste offset

        // if rotate is 0, then x = cpyx + (cpyx - pos1x) y = pos1y, z = cpyz + (cpyz - pos1z)
        switch(cur_rot){
            // 0 rotation
            case 0:
                // add to z subtract from x
                fp <<"/pos1 "<< (house.pos_x + start_x) - dx <<","<< py <<","<< (house.pos_z + start_z) + dz << endl;
                break;
            // 90 rotation
            case 1:
                // subtract from both
                fp <<"/pos1 "<< (house.pos_x + start_x) - dz <<","<< py <<","<< (house.pos_z + start_z) - dx << endl;
                break;
            // 180 rotation
            case 2:
                // add to x subtract from z
                fp <<"/pos1 "<< (house.pos_x + start_x) + dx <<","<< py <<","<< (house.pos_z + start_z) - dz << endl;
                break;
            // 270 rotation
            case 3:
                // add to both
                fp <<"/pos1 "<< (house.pos_x + start_x) + dz <<","<< py <<","<< (house.pos_z + start_z) + dx << endl;
                break;
        }

        // paste the house in no air blocks
        fp <<"/paste -a"<< endl;

        // reset new copy flag
        newCopyFlag = false;
    }

    // // add stop command (for WE CLI)
    // fp << "stop" << endl;

    fp.close();
    // output msg
    cout <<"City output as output.txt. Ensure /toggleplace is set to place at pos1"<<endl;
}

/*
* PrintGrid
*
* prints out the grid of the city to the specified image filepath
* if placeHouses was called, then the image will contain the houses,
* otherwise it will contain the city grid
*
*/
void City::printGrid(string const & filename)
{
    PNG input_cpy = (*input_img);

    // loop through cells
    for(auto cell: *cityCells){
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
                color = WTE_PX;
                break;
            // house
            case 2:
                color = RED_PX;
                break;
            // border
            case 3:
                color = OGE_PX;
                break;
            // line
            case 4:
                color = BLU_PX;
                break;
            // cliff
            case 5:
                color = GRN_PX;
                break;
        }
        // if(cell->isCliff) color = GRN_PX;
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

    // input_cpy.writeToFile(filename);

    // draw houses on top of grid
    if(placeHousesCalled){
        for(auto house:*houseList){
            int32_t width = getCityHouseWidth(house);
            int32_t x_width = 0;
            int32_t z_width = 0;
            int32_t x_corner = 0;
            int32_t z_corner = 0;
            // if its a corner house dont care about rotation
            // do if line goes from left to right ("H")
            if(!(house->rotation)){
                x_width = width;
                z_width = grid_cell_size;
            }
            // do if line goes from top to bottom ("I")
            else{
                x_width = grid_cell_size;
                z_width = width;
            }

            x_corner = house->pos_x - (((x_width - 1) >> 1) + 1);
            z_corner = house->pos_z - (((z_width - 1) >> 1) + 1);

            int x_max = x_width + x_corner;
            int z_max = z_width + z_corner;

            for(int i = z_corner; i < z_max; i++){
                for(int j = x_corner; j < x_max; j++){
                    // get current pixel
                    HSLAPixel & pixel = input_cpy.getPixel(j, i);
                    if(i == z_corner || j == x_corner || i == (z_max-1) || j == (x_max-1)){
                        if(width == 1){
                            pixel = GRY_PX;
                        }
                        else{
                            pixel = BLK_PX;
                        }
                    }
                    else{
                        pixel = BRN_PX;
                    }
                }
            }
        }
    }
    input_cpy.writeToFile(filename);
}

/*
* getCityHouseWidth
*
* returns the width of input cityHouse
*
*/
int32_t City::getCityHouseWidth(cityHouse* house)
{
    return(house->house_ptr->width);
}

/*
* getCityHouseType
*
* returns the type of input cityHouse
*
*/
int32_t City::getCityHouseType(cityHouse* house)
{
    return(house->house_ptr->type);
}

/*
* Destructor
*
* frees memory
*
*/
City::~City(){
    // free all cityCells
    for(int i = 0; i < gridLength; i++){
        for(int j = 0; j < gridWidth; j++){
            delete grid[i][j];
        }
    }

    // free linelist
    for(auto line:*lineList){
        delete line;
    }
    delete lineList;

    // free cityCells list
    delete cityCells;

    // free house list
    for(auto house:*houseList){
        delete house;
    }
    delete houseList;
}